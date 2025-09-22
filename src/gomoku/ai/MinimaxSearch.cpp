// MinimaxSearch.cpp
#include "gomoku/ai/MinimaxSearch.hpp"
#include "gomoku/ai/CandidateGenerator.hpp"
#include "gomoku/core/Board.hpp"
#include "gomoku/core/Logger.hpp"
#include <algorithm>
#include <limits>

namespace gomoku {

namespace {
    struct PatternConfig {
        int win;
        int open4;
        int half4;
        int open3;
        int half3;
        int open2;
        int half2;
        int single;
    };

    // Values copied from the original if/else ladders (evaluateOneDir used 500000 win,
    // quickScoreMove used 900000). Other values are identical.
    constexpr PatternConfig EvalScores { 500000, 120000, 30000, 12000, 3000, 1000, 300, 20 };
    constexpr PatternConfig OrderScores { 900000, 120000, 30000, 12000, 3000, 1000, 300, 20 };

    inline int patternScore(int len, int open, const PatternConfig& cfg)
    {
        if (len >= 5)
            return cfg.win;
        if (len == 4)
            return (open == 2 ? cfg.open4 : (open == 1 ? cfg.half4 : cfg.single));
        if (len == 3)
            return (open == 2 ? cfg.open3 : (open == 1 ? cfg.half3 : cfg.single));
        if (len == 2)
            return (open == 2 ? cfg.open2 : (open == 1 ? cfg.half2 : cfg.single));
        return cfg.single; // isolated stone or closed sequences
    }

    inline int scorePattern(bool orderingContext, int len, int open)
    {
        return patternScore(len, open, orderingContext ? OrderScores : EvalScores);
    }

    inline int manhattan(int x, int y, int cx, int cy)
    {
        return (x > cx ? x - cx : cx - x) + (y > cy ? y - cy : cy - y);
    }
} // namespace

// Note: cellOf and other are now available as playerToCell and opponent in Types.hpp
std::optional<Move> MinimaxSearch::bestMove(Board& board, const RuleSet& rules, SearchStats* stats)
{
    LOG_INFO("MinimaxSearch: Starting search for best move for "
        + std::string(board.toPlay() == Player::Black ? "Black" : "White"));

    if (stats)
        *stats = SearchStats {};

    // Ouverture: centre si plateau vide
    bool empty = true;
    for (uint8_t y = 0; y < BOARD_SIZE && empty; ++y)
        for (uint8_t x = 0; x < BOARD_SIZE; ++x)
            if (board.at(x, y) != Cell::Empty) {
                empty = false;
                break;
            }
    if (empty) {
        LOG_DEBUG("MinimaxSearch: Empty board - center opening move");
        Move c { { (uint8_t)(BOARD_SIZE / 2), (uint8_t)(BOARD_SIZE / 2) }, board.toPlay() };
        if (stats) {
            stats->nodes = 1;
            stats->depthReached = 0;
            stats->timeMs = 0;
            stats->principalVariation = { c };
        }
        return c;
    }

    LOG_DEBUG("MinimaxSearch: Initialization - Budget: " + std::to_string(cfg.timeBudgetMs) + "ms, TTBytes: " + std::to_string(cfg.ttBytes));

    // Populate TT from data if available
    budgetMs = cfg.timeBudgetMs;
    t0 = std::chrono::steady_clock::now();
    timeUp = false;
    visited = 0;
    tt.resizeBytes(cfg.ttBytes);
    nodeCap = cfg.nodeCap;

    std::optional<Move> best {};
    int bestScore = std::numeric_limits<int>::min();

    // Fallback rapide: premier légal (cas extrême)
    std::vector<Move> legals = board.legalMoves(board.toPlay(), rules);
    if (legals.empty()) {
        LOG_WARNING("MinimaxSearch: No legal moves available!");
        if (stats) {
            stats->nodes = 0;
            stats->depthReached = 0;
            stats->principalVariation.clear();
        }
        return std::nullopt;
    } else {
        LOG_DEBUG("MinimaxSearch: " + std::to_string(legals.size()) + " legal moves available");
        best = legals.front();
        bestScore = -1'000'000;
        if (stats) {
            stats->principalVariation = { *best };
        }
    }

    // Iterative deepening
    int maxDepth = std::max(2, cfg.maxDepthHint);
    LOG_DEBUG("MinimaxSearch: Iterative deepening up to depth " + std::to_string(maxDepth));
    for (int depth = 2; depth <= maxDepth; ++depth) {
        if (expired()) {
            timeUp = true;
            break;
        }
        LOG_DEBUG("MinimaxSearch: Searching at depth " + std::to_string(depth));
        auto res = alphabeta(board, rules, depth,
            std::numeric_limits<int>::min() / 2,
            std::numeric_limits<int>::max() / 2,
            board.toPlay(), stats);
        if (timeUp) {
            LOG_DEBUG("MinimaxSearch: Time expired at depth " + std::to_string(depth));
            break;
        }
        if (res.move) {
            best = res.move;
            bestScore = res.score;
            LOG_DEBUG("MinimaxSearch: Depth " + std::to_string(depth) + " - Score: " + std::to_string(bestScore)
                + " - Move: (" + std::to_string(res.move->pos.x) + "," + std::to_string(res.move->pos.y) + ")");
            if (stats) {
                stats->depthReached = depth;
                stats->principalVariation = { *res.move };
            }
            if (bestScore > 800000) {
                LOG_DEBUG("MinimaxSearch: Winning score detected (" + std::to_string(bestScore) + "), early termination");
                break;
            }
        }
    }

    if (stats) {
        using namespace std::chrono;
        stats->timeMs = (int)duration_cast<milliseconds>(steady_clock::now() - t0).count();
        if (best) {
            LOG_INFO("MinimaxSearch: Search completed - Time: " + std::to_string(stats->timeMs) + "ms, "
                + "Nodes: " + std::to_string(stats->nodes) + ", Depth: " + std::to_string(stats->depthReached)
                + " - Final move: (" + std::to_string(best->pos.x) + "," + std::to_string(best->pos.y) + ")");
        } else {
            LOG_WARNING("MinimaxSearch: Search completed without move found - Time: " + std::to_string(stats->timeMs) + "ms");
        }
    }
    return best;
}

// ---------------- Alpha-Beta ----------------
MinimaxSearch::ABResult MinimaxSearch::alphabeta(Board& b, const RuleSet& rules, int depth, int alpha, int beta, Player maxPlayer, SearchStats* stats)
{
    if (stats)
        ++stats->nodes;
    ++visited;
    if (expired()) {
        timeUp = true;
        return { evaluate(b, maxPlayer), std::nullopt };
    }

    uint64_t key = b.zobristKey();
    int alpha0 = alpha;
    Move ttBest {};
    bool haveTTBest = false;

    if (auto e = tt.probe(key)) {
        if (e->key == key && e->depth >= depth) {
            if (stats)
                ++stats->ttHits;
            if (e->flag == TranspositionTable::Flag::Exact)
                return { e->score, e->best };
            if (e->flag == TranspositionTable::Flag::Lower)
                alpha = std::max(alpha, e->score);
            else
                beta = std::min(beta, e->score);
            if (alpha >= beta)
                return { e->score, e->best };
        }
        if (e->key == key) {
            ttBest = e->best;
            haveTTBest = true;
        }
    }

    // Fin de partie ?
    if (b.status() != GameStatus::Ongoing) {
        int val = 0;
        if (b.status() == GameStatus::WinByAlign || b.status() == GameStatus::WinByCapture) {
            Player winner = opponent(b.toPlay());
            val = (winner == maxPlayer) ? 1'000'000 : -1'000'000;
        }
        return { val, std::nullopt };
    }

    if (depth == 0) {
        // Au lieu d'évaluer directement, utiliser la recherche de quiescence
        if (cfg.enableQuiescence) {
            return quiescence(b, rules, alpha, beta, maxPlayer, stats);
        } else {
            int val = evaluate(b, maxPlayer);
            return { val, std::nullopt };
        }
    }

    Player toPlay = b.toPlay();
    auto moves = orderedMoves(b, rules, toPlay);
    if (haveTTBest) {
        for (size_t i = 0; i < moves.size(); ++i) {
            if (moves[i].pos.x == ttBest.pos.x && moves[i].pos.y == ttBest.pos.y) {
                std::swap(moves[0], moves[i]);
                break;
            }
        }
    }
    if (moves.empty())
        return { 0, std::nullopt };

    std::optional<Move> bestMove {};

    if (toPlay == maxPlayer) {
        int best = std::numeric_limits<int>::min();
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            b.undo();
            if (timeUp)
                break;
            if (child.score > best) {
                best = child.score;
                bestMove = m;
            }
            alpha = std::max(alpha, child.score);
            if (alpha >= beta)
                break;
        }
        // TT store
        auto flag = TranspositionTable::Flag::Exact;
        if (best <= alpha0)
            flag = TranspositionTable::Flag::Upper;
        else if (best >= beta)
            flag = TranspositionTable::Flag::Lower;
        tt.store(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    } else {
        int best = std::numeric_limits<int>::max();
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            b.undo();
            if (timeUp)
                break;
            if (child.score < best) {
                best = child.score;
                bestMove = m;
            }
            beta = std::min(beta, child.score);
            if (beta <= alpha)
                break;
        }
        auto flag = TranspositionTable::Flag::Exact;
        if (best <= alpha0)
            flag = TranspositionTable::Flag::Upper;
        else if (best >= beta)
            flag = TranspositionTable::Flag::Lower;
        tt.store(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    }
}

// ---------------- Move ordering (léger) ----------------
std::vector<Move> MinimaxSearch::orderedMoves(Board& b, const RuleSet& rules, Player toPlay) const
{
    (void)rules;
    CandidateConfig cc;
    auto ms = CandidateGenerator::generate(b, rules, toPlay, cc);
    if (ms.size() <= 1) {
        LOG_DEBUG("MinimaxSearch: " + std::to_string(ms.size()) + " candidate(s) - no sorting needed");
        return ms;
    }

    LOG_DEBUG("MinimaxSearch: Sorting " + std::to_string(ms.size()) + " candidates by heuristic score");

    struct Sc {
        Move m;
        int s;
    };
    std::vector<Sc> scored;
    scored.reserve(ms.size());

    for (size_t i = 0; i < ms.size(); ++i) {
        if (expired()) {
            for (size_t j = i; j < ms.size(); ++j)
                scored.push_back({ ms[j], -100 });
            break;
        }
        const auto& m = ms[i];
        int s = quickScoreMove(b, toPlay, m.pos.x, m.pos.y);
        scored.push_back({ m, s });
    }

    std::stable_sort(scored.begin(), scored.end(),
        [](const Sc& lhs, const Sc& rhs) { return lhs.s > rhs.s; });

    std::vector<Move> out;
    out.reserve(scored.size());
    for (auto& e : scored)
        out.push_back(e.m);
    return out;
}

// Score local rapide sans play()
int MinimaxSearch::quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const
{
    const Cell me = (toPlay == Player::Black ? Cell::Black : Cell::White);
    const Cell opp = (me == Cell::Black ? Cell::White : Cell::Black);
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    if (b.at(x, y) != Cell::Empty)
        return -1'000'000;

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    int score = 0;
    score -= 3 * manhattan(x, y, BOARD_SIZE / 2, BOARD_SIZE / 2);

    for (int d = 0; d < 4; ++d) {
        int len = 1;
        bool openA = false, openB = false;

        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        int open = (openA ? 1 : 0) + (openB ? 1 : 0);
        // Ordering context uses the higher WIN constant (900000) while preserving
        // relative threat ordering; see helper table above.
        score += scorePattern(true /* ordering context */, len, open);
    }

    // opportunité de capture XOOX
    for (int d = 0; d < 4; ++d) {
        int x1 = (int)x + DX[d], y1 = (int)y + DY[d];
        int x2 = (int)x + 2 * DX[d], y2 = (int)y + 2 * DY[d];
        int x3 = (int)x + 3 * DX[d], y3 = (int)y + 3 * DY[d];
        if (inside(x3, y3) && b.at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == opp && b.at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == opp && b.at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == me)
            score += 4000;

        int X1 = (int)x - DX[d], Y1 = (int)y - DY[d];
        int X2 = (int)x - 2 * DX[d], Y2 = (int)y - 2 * DY[d];
        int X3 = (int)x - 3 * DX[d], Y3 = (int)y - 3 * DY[d];
        if (inside(X3, Y3) && b.at(static_cast<uint8_t>(X1), static_cast<uint8_t>(Y1)) == opp && b.at(static_cast<uint8_t>(X2), static_cast<uint8_t>(Y2)) == opp && b.at(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) == me)
            score += 4000;
    }

    return score;
}

// ---------------- Evaluation ----------------
int MinimaxSearch::evaluate(const Board& b, Player pov) const
{
    auto scoreSide = [&](Player p) -> int {
        int sum = 0;
        Cell who = playerToCell(p);
        for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
            for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
                if (b.at(x, y) != who)
                    continue;
                sum += evaluateOneDir(b, x, y, who, 1, 0);
                sum += evaluateOneDir(b, x, y, who, 0, 1);
                sum += evaluateOneDir(b, x, y, who, 1, 1);
                sum += evaluateOneDir(b, x, y, who, 1, -1);
            }
        }
        auto [bp, wp] = b.capturedPairs();
        int pairs = (p == Player::Black) ? (bp - wp) : (wp - bp);
        sum += pairs * 3000;
        return sum;
    };

    int me = scoreSide(pov);
    int opp = scoreSide(opponent(pov));
    return me - opp;
}

int MinimaxSearch::evaluateOneDir(const Board& b, uint8_t x, uint8_t y, Cell who, int dx, int dy) const
{
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    int X = x, Y = y, len = 1;
    int x1 = X - dx, y1 = Y - dy;
    while (inside(x1, y1) && b.at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == who) {
        ++len;
        x1 -= dx;
        y1 -= dy;
    }
    bool openA = inside(x1, y1) && b.at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == Cell::Empty;

    int x2 = X + dx, y2 = Y + dy;
    while (inside(x2, y2) && b.at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == who) {
        ++len;
        x2 += dx;
        y2 += dy;
    }
    bool openB = inside(x2, y2) && b.at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == Cell::Empty;

    int open = (openA ? 1 : 0) + (openB ? 1 : 0);
    return scorePattern(false /* evaluation context */, len, open);
}

// ---------------- Recherche de quiescence ----------------
MinimaxSearch::ABResult MinimaxSearch::quiescence(Board& b, const RuleSet& rules, int alpha, int beta, Player maxPlayer, SearchStats* stats, int qDepth)
{
    if (stats)
        ++stats->nodes;
    ++visited;

    if (expired()) {
        timeUp = true;
        return { evaluate(b, maxPlayer), std::nullopt };
    }

    // Limiter la profondeur de quiescence
    if (qDepth >= cfg.maxQuiescenceDepth) {
        return { evaluate(b, maxPlayer), std::nullopt };
    }

    // Évaluation statique de la position
    int standPat = evaluate(b, maxPlayer);

    // Fin de partie ?
    if (b.status() != GameStatus::Ongoing) {
        int val = 0;
        if (b.status() == GameStatus::WinByAlign || b.status() == GameStatus::WinByCapture) {
            Player winner = opponent(b.toPlay());
            val = (winner == maxPlayer) ? 1'000'000 : -1'000'000;
        }
        return { val, std::nullopt };
    }

    Player toPlay = b.toPlay();

    // Si la position est calme, retourner l'évaluation statique
    if (isQuiet(b, rules, toPlay)) {
        return { standPat, std::nullopt };
    }

    // Beta cutoff
    if (toPlay == maxPlayer && standPat >= beta) {
        return { beta, std::nullopt };
    }

    // Alpha cutoff
    if (toPlay != maxPlayer && standPat <= alpha) {
        return { alpha, std::nullopt };
    }

    // Mise à jour de alpha pour le joueur maximisant
    if (toPlay == maxPlayer) {
        alpha = std::max(alpha, standPat);
    } else {
        beta = std::min(beta, standPat);
    }

    // Générer seulement les mouvements tactiques
    auto moves = tacticalMoves(b, rules, toPlay);
    if (moves.empty()) {
        return { standPat, std::nullopt };
    }

    std::optional<Move> bestMove {};

    if (toPlay == maxPlayer) {
        int best = standPat;
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = quiescence(b, rules, alpha, beta, maxPlayer, stats, qDepth + 1);
            b.undo();
            if (timeUp)
                break;
            if (child.score > best) {
                best = child.score;
                bestMove = m;
            }
            alpha = std::max(alpha, child.score);
            if (alpha >= beta)
                break;
        }
        return { best, bestMove };
    } else {
        int best = standPat;
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = quiescence(b, rules, alpha, beta, maxPlayer, stats, qDepth + 1);
            b.undo();
            if (timeUp)
                break;
            if (child.score < best) {
                best = child.score;
                bestMove = m;
            }
            beta = std::min(beta, child.score);
            if (beta <= alpha)
                break;
        }
        return { best, bestMove };
    }
}

// ---------------- Détection des mouvements tactiques ----------------
std::vector<Move> MinimaxSearch::tacticalMoves(Board& b, const RuleSet& rules, Player toPlay) const
{
    std::vector<Move> allMoves = orderedMoves(b, rules, toPlay);
    std::vector<Move> tactical;

    // Filtrer les mouvements tactiquement importants
    for (const auto& move : allMoves) {
        if (isTacticalMove(b, move.pos.x, move.pos.y, toPlay)) {
            tactical.push_back(move);
        }
        // Limiter le nombre de mouvements tactiques pour éviter l'explosion
        if (tactical.size() >= 8)
            break;
    }

    return tactical;
}

bool MinimaxSearch::isTacticalMove(const Board& b, uint8_t x, uint8_t y, Player toPlay) const
{
    if (b.at(x, y) != Cell::Empty)
        return false;

    const Cell me = (toPlay == Player::Black ? Cell::Black : Cell::White);
    const Cell opp = (me == Cell::Black ? Cell::White : Cell::Black);

    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    for (int d = 0; d < 4; ++d) {
        // Vérifier les menaces de victoire (alignements de 4)
        int len = 1;
        bool openA = false, openB = false;

        // Extension vers l'arrière
        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        // Extension vers l'avant
        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        int open = (openA ? 1 : 0) + (openB ? 1 : 0);

        // Menace de victoire directe (4 en ligne)
        if (len >= 4) {
            return true;
        }

        // Menace forte (3 ouvert ou 4 semi-ouvert)
        if ((len == 3 && open == 2) || (len == 4 && open >= 1)) {
            return true;
        }

        // Vérifier les blocages de menaces adverses
        len = 1;
        openA = openB = false;

        // Extension vers l'arrière pour l'adversaire
        xx = (int)x - DX[d];
        yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == opp) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        // Extension vers l'avant pour l'adversaire
        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == opp) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && b.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        open = (openA ? 1 : 0) + (openB ? 1 : 0);

        // Blocage de menace de victoire adverse
        if (len >= 4) {
            return true;
        }

        // Blocage de menace forte adverse
        if ((len == 3 && open == 2) || (len == 4 && open >= 1)) {
            return true;
        }

        // Vérifier les opportunities de capture (pattern XOOY)
        int x1 = (int)x + DX[d], y1 = (int)y + DY[d];
        int x2 = (int)x + 2 * DX[d], y2 = (int)y + 2 * DY[d];
        int x3 = (int)x + 3 * DX[d], y3 = (int)y + 3 * DY[d];
        if (inside(x3, y3) && b.at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == opp && b.at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == opp && b.at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == me) {
            return true;
        }

        int X1 = (int)x - DX[d], Y1 = (int)y - DY[d];
        int X2 = (int)x - 2 * DX[d], Y2 = (int)y - 2 * DY[d];
        int X3 = (int)x - 3 * DX[d], Y3 = (int)y - 3 * DY[d];
        if (inside(X3, Y3) && b.at(static_cast<uint8_t>(X1), static_cast<uint8_t>(Y1)) == opp && b.at(static_cast<uint8_t>(X2), static_cast<uint8_t>(Y2)) == opp && b.at(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) == me) {
            return true;
        }
    }

    return false;
}

bool MinimaxSearch::isQuiet(const Board& b, const RuleSet& rules, Player toPlay) const
{
    // Une position est calme s'il n'y a pas de mouvements tactiquement critiques
    // On fait une copie temporaire du plateau pour éviter la modification
    Board temp = b;
    auto tacticals = tacticalMoves(temp, rules, toPlay);
    return tacticals.empty();
}

} // namespace gomoku
