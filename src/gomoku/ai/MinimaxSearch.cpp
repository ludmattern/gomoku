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
        int split4; // X_XXX, XX_XX (4 avec gap)
        int open3;
        int half3;
        int double3; // Deux open-3 créés simultanément
        int fork3; // Fourchette de 3
        int open2;
        int half2;
        int double2; // Deux open-2 créés simultanément
        int single;
        int capture_threat; // Menace de capture XOOX
        int double_threat; // Menaces multiples
    };

    // Values copied from the original if/else ladders (evaluateOneDir used 500000 win,
    // quickScoreMove used 900000). Other values are identical.
    constexpr PatternConfig EvalScores {
        500000, // win
        120000, // open4
        30000, // half4
        80000, // split4 - très dangereux car difficile à bloquer
        12000, // open3
        3000, // half3
        25000, // double3 - force généralement la victoire
        18000, // fork3 - fourchette de 3
        1000, // open2
        300, // half2
        2500, // double2
        20, // single
        8000, // capture_threat
        15000 // double_threat
    };

    constexpr PatternConfig OrderScores {
        900000, // win
        120000, // open4
        30000, // half4
        85000, // split4
        12000, // open3
        3000, // half3
        30000, // double3
        20000, // fork3
        1000, // open2
        300, // half2
        3000, // double2
        20, // single
        10000, // capture_threat
        18000 // double_threat
    };

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
    clearPV();

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
            board.toPlay(), stats, /*ply=*/0);
        if (timeUp) {
            break;
        }
        if (res.move) {
            best = res.move;
            bestScore = res.score;
            if (stats) {
                stats->depthReached = depth;
                // Export PV line
                stats->principalVariation.clear();
                for (int i = 0; i < pvLen[0]; ++i) {
                    const auto& mv = pvTable[0][i];
                    if (mv.pos.x != 255) {
                        stats->principalVariation.push_back(mv);
                    }
                }
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
MinimaxSearch::ABResult MinimaxSearch::alphabeta(Board& b, const RuleSet& rules, int depth, int alpha, int beta, Player maxPlayer, SearchStats* stats, int ply)
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
    int beta0 = beta;
    Move ttBest {};
    bool haveTTBest = false;

    if (auto e = tt.probe(key)) {
        if (e->key == key && e->depth >= depth) {
            if (stats)
                ++stats->ttHits;
            if (e->flag == TranspositionTable::Flag::Exact)
                return { e->score, (e->best.pos.x == 255 ? std::optional<Move> {} : std::optional<Move> { e->best }) };
            if (e->flag == TranspositionTable::Flag::Lower)
                alpha = std::max(alpha, e->score);
            else
                beta = std::min(beta, e->score);
            if (alpha >= beta)
                return { e->score, (e->best.pos.x == 255 ? std::optional<Move> {} : std::optional<Move> { e->best }) };
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
    // Use ply (distance from root) for killer moves stability across ID iterations
    auto moves = orderedMoves(b, rules, toPlay, ply);
    if (haveTTBest && ttBest.pos.x != 255) {
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

        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;

            int newDepth = depth - 1;

            // S'assurer que la profondeur reste positive
            newDepth = std::max(0, newDepth);

            auto child = alphabeta(b, rules, newDepth, alpha, beta, maxPlayer, stats, ply + 1);

            b.undo();
            if (timeUp)
                break;
            if (child.score > best) {
                best = child.score;
                bestMove = m;
                setPVMove(ply, m);
                copyChildPVUp(ply);
            }
            alpha = std::max(alpha, child.score);
            if (alpha >= beta) {
                // Beta cutoff - store killer by ply and update history by remaining depth
                storeKiller(ply, m);
                updateHistory(m, depth);
                break;
            }

            moveIndex++;
        }
        // TT store
        auto flag = TranspositionTable::Flag::Exact;
        if (best <= alpha0)
            flag = TranspositionTable::Flag::Upper;
        else if (best >= beta0)
            flag = TranspositionTable::Flag::Lower;
        tt.store(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    } else {
        int best = std::numeric_limits<int>::max();
        int moveIndex = 0;

        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;

            int newDepth = depth - 1;

            // S'assurer que la profondeur reste positive
            newDepth = std::max(0, newDepth);

            auto child = alphabeta(b, rules, newDepth, alpha, beta, maxPlayer, stats, ply + 1);

            b.undo();
            if (timeUp)
                break;
            if (child.score < best) {
                best = child.score;
                bestMove = m;
                setPVMove(ply, m);
                copyChildPVUp(ply);
            }
            beta = std::min(beta, child.score);
            if (beta <= alpha) {
                // Alpha cutoff - store killer by ply and update history by remaining depth
                storeKiller(ply, m);
                updateHistory(m, depth);
                break;
            }

            moveIndex++;
        }
        auto flag = TranspositionTable::Flag::Exact;
        if (best <= alpha0)
            flag = TranspositionTable::Flag::Upper;
        else if (best >= beta0)
            flag = TranspositionTable::Flag::Lower;
        tt.store(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    }
}

// --- Move ordering (optimisé) ---
std::vector<Move> MinimaxSearch::orderedMoves(Board& b, const RuleSet& rules, Player toPlay, int ply) const
{
    (void)rules;
    CandidateConfig cc;
    auto ms = CandidateGenerator::generate(b, rules, toPlay, cc);
    if (ms.size() <= 1)
        return ms;

    struct Sc {
        Move m;
        int s;
    };
    std::vector<Sc> scored;
    scored.reserve(ms.size());

    // Early bucket pour les victoires en 1
    std::vector<Move> instantWins;
    instantWins.reserve(4);

    for (const auto& m : ms) {
        if (expired())
            return ms; // fallback si budget brûle

        // Offense + Défense : on classe par le maximum des deux urgences
        const int sOff = quickScoreMove(b, toPlay, m.pos.x, m.pos.y);
        const int sDef = quickScoreDefense(b, toPlay, m.pos.x, m.pos.y);
        int s = std::max(sOff, sDef);

        // (Optionnel) motifs avancés : réactiver si utile
        // s += evaluateAdvancedPatterns(b, m.pos.x, m.pos.y, toPlay, /*orderingContext=*/true);

        // Killers / History
        if (isKillerMove(ply, m))
            s += 1'000'000;
        s += getHistoryScore(m);

        scored.push_back({ m, s });

        if (sOff >= OrderScores.win) {
            instantWins.push_back(m);
        }
    }

    // Si win immédiate détectée, on la joue d'abord (l'αβ coupera le reste)
    if (!instantWins.empty())
        return instantWins;

    std::sort(scored.begin(), scored.end(), [](const Sc& lhs, const Sc& rhs) { return lhs.s > rhs.s; });

    std::vector<Move> out;
    out.reserve(scored.size());
    for (auto& e : scored)
        out.push_back(e.m);
    return out;
}

// --- Score local rapide (contexte ordering) ---
int MinimaxSearch::quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const
{
    const Cell me = (toPlay == Player::Black ? Cell::Black : Cell::White);
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    if (b.at(x, y) != Cell::Empty)
        return -1'000'000;

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    int score = 0;
    // Léger recentrage (manhattan)
    score -= 1 * manhattan(x, y, BOARD_SIZE / 2, BOARD_SIZE / 2);

    // Motifs simples par direction (orderingContext=true)
    for (int d = 0; d < 4; ++d) {
        int len = 1;
        bool openA = false, openB = false;

        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == me) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == Cell::Empty;

        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == me) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == Cell::Empty;

        const int open = (openA ? 1 : 0) + (openB ? 1 : 0);
        score += scorePattern(true /* ordering context */, len, open);
    }

    return score;
}

// --- Score défensif rapide (bloque menaces adverses) ---
int MinimaxSearch::quickScoreDefense(const Board& b, Player meP, uint8_t x, uint8_t y) const
{
    if (b.at(x, y) != Cell::Empty)
        return -1'000'000;

    const Cell me = (meP == Player::Black ? Cell::Black : Cell::White);
    const Cell op = (me == Cell::Black ? Cell::White : Cell::Black);

    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };
    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    int s = 0;

    // léger recentrage mais plus faible que l'offense
    s -= 1 * manhattan(x, y, BOARD_SIZE / 2, BOARD_SIZE / 2);

    for (int d = 0; d < 4; ++d) {
        // Chaîne adverse autour de (x,y) si on POSAIT ici (blocage)
        int ao = 0, bo = 0;

        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == op) {
            ++ao;
            xx -= DX[d];
            yy -= DY[d];
        }
        bool openA = inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == Cell::Empty;

        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == op) {
            ++bo;
            xx += DX[d];
            yy += DY[d];
        }
        bool openB = inside(xx, yy) && b.at((uint8_t)xx, (uint8_t)yy) == Cell::Empty;

        const int len = 1 + ao + bo; // longueur si on bloque ici
        const int open = (openA ? 1 : 0) + (openB ? 1 : 0);

        // Priorise les blocs vitaux
        if (len >= 5)
            s += OrderScores.win; // bloque 5 (critique)
        else if (len == 4 && open >= 1)
            s += OrderScores.open4; // bloque 4
        else if (len == 3 && open == 2)
            s += OrderScores.open3; // bloque open-3
    }
    return s;
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

                // Note: patterns avancés réservés à l'ordering pour performance
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

    // Head-only: ne compter la séquence que si (x - dx, y - dy) n'est pas la même couleur
    int prevX = static_cast<int>(x) - dx;
    int prevY = static_cast<int>(y) - dy;
    if (inside(prevX, prevY) && b.at(static_cast<uint8_t>(prevX), static_cast<uint8_t>(prevY)) == who) {
        return 0; // pas tête de séquence, on ne recompte pas
    }

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
    for (int x = 0; x < 19; ++x) {
        for (int y = 0; y < 19; ++y) {
            historyTable[x][y] = 0;
        }
    }
}

void MinimaxSearch::storeKiller(int ply, const Move& move)
{
    if (ply < 0 || ply >= MAX_DEPTH)
        return;

    // Éviter les doublons - si le mouvement est déjà le premier killer, ne rien faire
    if (killerMoves[ply][0].pos.x == move.pos.x && killerMoves[ply][0].pos.y == move.pos.y) {
        return;
    }

    // Décaler les killers : killer[1] = killer[0], killer[0] = nouveau
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = move;
}

bool MinimaxSearch::isKillerMove(int ply, const Move& move) const
{
    if (ply < 0 || ply >= MAX_DEPTH)
        return false;

    for (int i = 0; i < MAX_KILLERS; ++i) {
        if (killerMoves[ply][i].pos.x == move.pos.x && killerMoves[ply][i].pos.y == move.pos.y) {
            return true;
        }
    }
    return false;
}

void MinimaxSearch::updateHistory(const Move& move, int depth)
{
    if (move.pos.x < 19 && move.pos.y < 19) {
        // Plus la coupure est profonde, plus elle est importante
        historyTable[move.pos.x][move.pos.y] += depth * depth;
    }
}

int MinimaxSearch::getHistoryScore(const Move& move) const
{
    if (move.pos.x < 19 && move.pos.y < 19) {
        return historyTable[move.pos.x][move.pos.y];
    }
    return 0;
}

// === PV helpers ===

void MinimaxSearch::clearPV()
{
    for (int i = 0; i < MAX_DEPTH; ++i) {
        pvLen[i] = 0;
        for (int j = 0; j < MAX_DEPTH; ++j) {
            pvTable[i][j] = Move { { 255, 255 }, Player::Black };
        }
    }
}

void MinimaxSearch::setPVMove(int ply, const Move& m)
{
    if (ply < 0 || ply >= MAX_DEPTH)
        return;
    pvTable[ply][0] = m;
    pvLen[ply] = 1;
}

void MinimaxSearch::copyChildPVUp(int ply)
{
    if (ply < 0 || ply + 1 >= MAX_DEPTH)
        return;
    int childLen = pvLen[ply + 1];
    for (int i = 0; i < childLen && i + 1 < MAX_DEPTH; ++i) {
        pvTable[ply][i + 1] = pvTable[ply + 1][i];
    }
    pvLen[ply] = 1 + childLen;
}

} // namespace gomoku
