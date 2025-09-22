// MinimaxSearch.cpp
#include "gomoku/ai/MinimaxSearch.hpp"
#include "gomoku/ai/CandidateGenerator.hpp"
#include "gomoku/core/Board.hpp"
#include "gomoku/core/Logger.hpp"
#include <algorithm>
#include <functional>
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

    // Réinitialiser les killer moves et l'historique pour cette nouvelle recherche
    clearKillersAndHistory();

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
    for (int depth = 2; depth <= maxDepth; ++depth) {
        if (expired()) {
            timeUp = true;
            break;
        }
        auto res = alphabeta(board, rules, depth,
            std::numeric_limits<int>::min() / 2,
            std::numeric_limits<int>::max() / 2,
            board.toPlay(), stats);
        if (timeUp) {
            break;
        }
        if (res.move) {
            best = res.move;
            bestScore = res.score;
            if (stats) {
                stats->depthReached = depth;
                stats->principalVariation = { *res.move };
            }
            if (bestScore > 800000) {
                break; // Early termination for winning positions
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
        // Utiliser la recherche de quiescence
        if (cfg.quiescence.enabled) {
            auto evaluateFunc = [this](const Board& board, Player player) {
                return evaluate(board, player);
            };
            auto qResult = quiescenceSearch_.search(b, rules, alpha, beta, maxPlayer, evaluateFunc, stats);
            return { qResult.score, qResult.move };
        } else {
            int val = evaluate(b, maxPlayer);
            return { val, std::nullopt };
        }
    }

    Player toPlay = b.toPlay();
    auto moves = orderedMoves(b, rules, toPlay, depth);
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
        int moveIndex = 0;
        bool isPVNode = (alpha != beta - 1); // Nœud PV si la fenêtre n'est pas nulle

        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;

            // Calculer la réduction LMR pour ce mouvement
            int reduction = calculateLMRReduction(depth, moveIndex, isPVNode);
            int newDepth = depth - 1 - reduction;

            // S'assurer que la profondeur reste positive
            newDepth = std::max(0, newDepth);

            auto child = alphabeta(b, rules, newDepth, alpha, beta, maxPlayer, stats);

            // Si LMR était appliqué et que le résultat est prometteur, refaire la recherche complète
            if (reduction > 0 && child.score > alpha && newDepth < depth - 1) {
                child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            }

            b.undo();
            if (timeUp)
                break;
            if (child.score > best) {
                best = child.score;
                bestMove = m;
                isPVNode = true; // Les mouvements suivants seront dans une fenêtre PV
            }
            alpha = std::max(alpha, child.score);
            if (alpha >= beta) {
                // Beta cutoff - stocker ce mouvement comme killer et mettre à jour l'historique
                storeKiller(depth, m);
                updateHistory(m, depth);
                break;
            }

            moveIndex++;
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
        int moveIndex = 0;
        bool isPVNode = (alpha != beta - 1); // Nœud PV si la fenêtre n'est pas nulle

        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;

            // Calculer la réduction LMR pour ce mouvement
            int reduction = calculateLMRReduction(depth, moveIndex, isPVNode);
            int newDepth = depth - 1 - reduction;

            // S'assurer que la profondeur reste positive
            newDepth = std::max(0, newDepth);

            auto child = alphabeta(b, rules, newDepth, alpha, beta, maxPlayer, stats);

            // Si LMR était appliqué et que le résultat est prometteur, refaire la recherche complète
            if (reduction > 0 && child.score < beta && newDepth < depth - 1) {
                child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            }

            b.undo();
            if (timeUp)
                break;
            if (child.score < best) {
                best = child.score;
                bestMove = m;
                isPVNode = true; // Les mouvements suivants seront dans une fenêtre PV
            }
            beta = std::min(beta, child.score);
            if (beta <= alpha) {
                // Alpha cutoff - stocker ce mouvement comme killer et mettre à jour l'historique
                storeKiller(depth, m);
                updateHistory(m, depth);
                break;
            }

            moveIndex++;
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
std::vector<Move> MinimaxSearch::orderedMoves(Board& b, const RuleSet& rules, Player toPlay, int depth) const
{
    (void)rules;
    CandidateConfig cc;
    auto ms = CandidateGenerator::generate(b, rules, toPlay, cc);
    if (ms.size() <= 1) {
        return ms;
    }

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
        int s = 0;

        // 1. Score de base : heuristique locale (tactique)
        s = quickScoreMove(b, toPlay, m.pos.x, m.pos.y);

        // 2. Bonus killers (priorité élevée)
        if (isKillerMove(depth, m)) {
            s += 1000000; // Priorité très élevée pour les killers
        }

        // 3. Bonus history (priorité moyenne)
        s += getHistoryScore(m); // Ne pas diviser, laisser son poids naturel

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

// === Killer Moves & History Heuristic ===

void MinimaxSearch::clearKillersAndHistory()
{
    // Initialiser les killer moves avec des mouvements invalides
    for (int depth = 0; depth < MAX_DEPTH; ++depth) {
        for (int i = 0; i < MAX_KILLERS; ++i) {
            killerMoves[depth][i] = Move { { 255, 255 }, Player::Black }; // Position invalide comme marqueur
        }
    }

    // Initialiser la table d'historique à 0
    for (int x = 0; x < 15; ++x) {
        for (int y = 0; y < 15; ++y) {
            historyTable[x][y] = 0;
        }
    }
}

void MinimaxSearch::storeKiller(int depth, const Move& move)
{
    if (depth < 0 || depth >= MAX_DEPTH)
        return;

    // Éviter les doublons - si le mouvement est déjà le premier killer, ne rien faire
    if (killerMoves[depth][0].pos.x == move.pos.x && killerMoves[depth][0].pos.y == move.pos.y) {
        return;
    }

    // Décaler les killers : killer[1] = killer[0], killer[0] = nouveau
    killerMoves[depth][1] = killerMoves[depth][0];
    killerMoves[depth][0] = move;
}

bool MinimaxSearch::isKillerMove(int depth, const Move& move) const
{
    if (depth < 0 || depth >= MAX_DEPTH)
        return false;

    for (int i = 0; i < MAX_KILLERS; ++i) {
        if (killerMoves[depth][i].pos.x == move.pos.x && killerMoves[depth][i].pos.y == move.pos.y) {
            return true;
        }
    }
    return false;
}

void MinimaxSearch::updateHistory(const Move& move, int depth)
{
    if (move.pos.x < 15 && move.pos.y < 15) {
        // Plus la coupure est profonde, plus elle est importante
        historyTable[move.pos.x][move.pos.y] += depth * depth;
    }
}

int MinimaxSearch::getHistoryScore(const Move& move) const
{
    if (move.pos.x < 15 && move.pos.y < 15) {
        return historyTable[move.pos.x][move.pos.y];
    }
    return 0;
}

int MinimaxSearch::calculateLMRReduction(int depth, int moveIndex, bool isPVNode) const
{
    // Pas de réduction si LMR est désactivé
    if (!cfg.lmrEnabled) {
        return 0;
    }

    // Pas de réduction si la profondeur est trop faible
    if (depth < cfg.lmrMinDepth) {
        return 0;
    }

    // Pas de réduction pour les premiers mouvements (supposés meilleurs)
    if (moveIndex < cfg.lmrMoveThreshold) {
        return 0;
    }

    // Réduction moins agressive pour les nœuds PV (Principal Variation)
    int reduction = cfg.lmrReduction;
    if (isPVNode) {
        reduction = std::max(1, reduction - 1);
    }

    // Réduction progressive : plus le mouvement est tard dans la liste, plus la réduction est forte
    int additionalReduction = (moveIndex - cfg.lmrMoveThreshold) / 4;
    reduction += additionalReduction;

    // Limiter la réduction pour éviter de trop réduire
    reduction = std::min(reduction, depth - 1);

    return reduction;
}

} // namespace gomoku
