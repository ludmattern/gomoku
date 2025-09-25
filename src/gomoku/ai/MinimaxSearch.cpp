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
    using Clock = std::chrono::steady_clock;

    inline void setStats(SearchStats* stats, Clock::time_point start, long long nodes, long long qnodes, int depth, int ttHits, const std::vector<Move>& pv)
    {
        if (!stats)
            return;
        stats->nodes = nodes;
        stats->qnodes = qnodes;
        stats->depthReached = depth;
        stats->timeMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
        stats->ttHits = ttHits;
        stats->principalVariation = pv;
    }

    // Generate root candidates with fallback to legal moves
    inline std::vector<Move> genRootCandidates(const Board& board, const RuleSet& rules, Player toPlay)
    {
        auto cands = CandidateGenerator::generate(board, rules, toPlay, CandidateConfig {});
        if (cands.empty())
            cands = board.legalMoves(toPlay, rules);
        return cands;
    }

} // namespace

// Note: cellOf and other are now available as playerToCell and opponent in Types.hpp
std::optional<Move> MinimaxSearch::bestMove(Board& board, const RuleSet& rules, SearchStats* stats)
{
    using namespace std::chrono;
    auto start = steady_clock::now();
    auto deadline = start + milliseconds(cfg.timeBudgetMs);
    SearchContext ctx { rules, deadline, stats, cfg.nodeCap };

    Player toPlay = board.toPlay();
    // Early terminal check
    int terminalScore = 0;
    if (isTerminal(board, /*ply*/ 0, terminalScore)) {
        setStats(stats, start, 0, 0, 0, 0, {});
        return std::nullopt;
    }

    std::vector<Move> candidates = genRootCandidates(board, rules, toPlay);

    if (candidates.empty()) {
        setStats(stats, start, 0, 0, 0, 0, {});
        return std::nullopt;
    }

    // 1) Immediate win shortcut (only if situation permits)
    if (auto iw = tryImmediateWinShortcut(board, rules, toPlay, candidates)) {
        setStats(stats, start, /*nodes*/ 1, /*qnodes*/ 0, /*depth*/ 1, /*ttHits*/ 0, { *iw });
        return iw;
    }

    // 2) Iterative deepening skeleton using the compact helper
    std::optional<Move> best;
    std::vector<Move> pv;
    long long nodes = 0;
    int ttHits = 0;
    int maxDepth = cfg.maxDepthHint;
    int bestScore = -INF;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (!runDepth(depth, board, rules, toPlay, candidates, best, bestScore, pv, nodes, ctx))
            break;
        setStats(stats, start, nodes, /*qnodes*/ 0, /*depth*/ depth, ttHits, pv);
    }

    if (best) {
        return best;
    }

    setStats(stats, start, 0, 0, 0, 0, {});
    return std::nullopt;
}

std::vector<Move> MinimaxSearch::orderedMovesPublic(const Board& board, const RuleSet& rules, Player toPlay) const
{
    auto moves = CandidateGenerator::generate(board, rules, toPlay, CandidateConfig {});
    if (moves.empty()) {
        moves = board.legalMoves(toPlay, rules);
    }
    return moves;
}

// --- Stubs for private methods declared in MinimaxSearch.hpp ---

// Négamax récursif (Gomoku) avec extensions possibles plus tard (alpha-bêta/PVS/TT).
// Rôle:
//  - Arrêt immédiat si état terminal (cinq alignés, victoire par captures, nul).
//  - En feuille (profondeur 0): renvoyer evaluate(...).
//  - Sinon: ordonner les coups via orderMoves(...) en privilégiant les menaces Gomoku
//    (gains en 1, parades de menaces adverses, créations de quatre ouverts, captures de paires critiques),
//    puis explorer récursivement (tryPlay → negamax → undo), construire la PV.
// TODO (palier 3): implémentation depth-limitée simple (sans alpha-bêta), puis ajouter alpha-bêta/PVS.
int MinimaxSearch::negamax(Board& board,
    int depth,
    int alpha,
    int beta,
    int ply,
    std::vector<Move>& pvOut,
    const SearchContext& ctx)
{
    // TODO: Terminal check, éval en feuille, génération + boucle enfants (tryPlay/undo), build PV.
    // TODO (plus tard): alpha-bêta/PVS, TT probe/store, extensions sur menaces (quatre ouvert, capture gagnante), LMR ciblé.
    (void)board;
    (void)depth;
    (void)alpha;
    (void)beta;
    (void)ply;
    (void)ctx;
    pvOut.clear();
    return 0;
}

// Recherche de quiétude (Gomoku):
//  - Stabilise l'évaluation en explorant uniquement les coups tactiques pertinents Gomoku:
//    • gains immédiats (faire 5), parades immédiates (bloquer 5/adversaire),
//    • créations/bloquages de quatre ouverts,
//    • captures de paires qui gagnent ou évitent une défaite par captures,
//    • éventuellement prolongations locales de menaces fortes.
//  - Évite d’explorer des coups calmes qui n’affectent pas les menaces en cours.
// TODO: stand-pat (éval statique), delta pruning adapté aux marges de menaces, génération coups tactiques.
int MinimaxSearch::qsearch(Board& board,
    int alpha,
    int beta,
    int ply,
    const SearchContext& ctx)
{
    // TODO: Stand pat, delta pruning, génération coups tactiques, tryPlay/undo, récursif, alpha/beta update.
    (void)board;
    (void)alpha;
    (void)beta;
    (void)ply;
    (void)ctx;
    return 0;
}

// Évaluation statique rapide d'une position (Gomoku):
//  - Patterns: cinq (win), overline (6+) (selon règles), quatre ouvert/fermé, trois ouverts, double-trois (interdit selon règles).
//  - Captures de paires: avantage/menace de capture, victoire par captures possible.
//  - Géométrie: centralité et densité locale autour du front de jeu (proximité des dernières pierres).
//  - Perspective: score positif si favorable au joueur 'perspective'.
// TODO: implémenter un score léger (captures + patterns basiques + centralité).
int MinimaxSearch::evaluate(const Board& board, Player perspective) const
{
    // Safety: terminal states are handled by isTerminal() in search, but keep neutral for draws here.
    if (board.status() == GameStatus::Draw)
        return 0;

    const Cell me = playerToCell(perspective);
    const Cell opp = playerToCell(opponent(perspective));

    int score = 0;

    // 1) Captures differential (pairs). Each pair is valuable tactically.
    const auto caps = board.capturedPairs();
    const int capDiff = (perspective == Player::Black) ? (caps.black - caps.white) : (caps.white - caps.black);
    constexpr int CAPTURE_PAIR_VALUE = 3000; // tuned later
    score += capDiff * CAPTURE_PAIR_VALUE;

    // 2) Centrality (manhattan distance to center). Encourages occupying the center early.
    constexpr int cx = BOARD_SIZE / 2;
    constexpr int cy = BOARD_SIZE / 2;
    constexpr int CENTER_BASE = 10; // max shells counted
    constexpr int CENTER_WEIGHT = 3; // per-shell weight multiplier
    int central = 0;
    const auto& occ = board.occupiedPositions();
    for (const auto& p : occ) {
        const int x = p.x, y = p.y;
        const Cell c = board.at(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
        const int md = std::abs(x - cx) + std::abs(y - cy);
        const int w = std::max(0, CENTER_BASE - md);
        if (c == me)
            central += w;
        else if (c == opp)
            central -= w;
    }
    score += central * CENTER_WEIGHT;

    // 2b) Front proximity: bias towards stones near the recent front (last 3 moves, weighted).
    {
        constexpr int FRONT_BASE = 6; // radius-like shells
        constexpr int FRONT_WEIGHT = 5; // final multiplier
        // Weights for last moves: most recent gets highest weight
        constexpr int W1 = 3, W2 = 2, W3 = 1; // sum = 6
        const auto recents = board.lastMoves(3);
        if (!recents.empty()) {
            int frontAccum = 0;
            int weightSum = 0;
            for (std::size_t i = 0; i < recents.size(); ++i) {
                const int wMove = (i == 0 ? W1 : (i == 1 ? W2 : W3));
                weightSum += wMove;
                const int lx = recents[i].pos.x;
                const int ly = recents[i].pos.y;
                int frontLocal = 0;
                for (const auto& p : occ) {
                    const int x = p.x, y = p.y;
                    const int md = std::abs(x - lx) + std::abs(y - ly);
                    if (md > FRONT_BASE)
                        continue;
                    const Cell c = board.at(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
                    const int w = FRONT_BASE - md;
                    if (c == me)
                        frontLocal += w;
                    else if (c == opp)
                        frontLocal -= w;
                }
                frontAccum += frontLocal * wMove;
            }
            // Divide by sum of move weights (6) to get an average-like effect
            score += (frontAccum / (W1 + W2 + W3)) * FRONT_WEIGHT;
        }
    }

    // 3) Pattern runs in 4 directions (open/closed 2/3/4, 5+).
    auto runValue = [](int len, int openEnds) -> int {
        if (len >= 5)
            return 100000; // effectively winning pattern (search should catch terminal earlier)
        switch (len) {
        case 4:
            return (openEnds >= 2) ? 10000 : 2500;
        case 3:
            return (openEnds >= 2) ? 600 : 150;
        case 2:
            return (openEnds >= 2) ? 80 : 20;
        case 1:
        default:
            return (openEnds >= 2) ? 10 : 2;
        }
    };

    constexpr int DIRS[4][2] = { { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 } };
    int patternScore = 0;
    for (const auto& p : occ) {
        const int x = p.x, y = p.y;
        const Cell c = board.at(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
        for (const auto& d : DIRS) {
            const int dx = d[0], dy = d[1];
            const int prevX = x - dx, prevY = y - dy;
            // Only start at the beginning of a run for this direction
            if (prevX >= 0 && prevX < BOARD_SIZE && prevY >= 0 && prevY < BOARD_SIZE) {
                if (board.at(static_cast<uint8_t>(prevX), static_cast<uint8_t>(prevY)) == c)
                    continue;
            }
            // Count run length
            int len = 0;
            int nx = x, ny = y;
            while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board.at(static_cast<uint8_t>(nx), static_cast<uint8_t>(ny)) == c) {
                ++len;
                nx += dx;
                ny += dy;
            }
            // Determine openness at both ends
            int openEnds = 0;
            if (prevX >= 0 && prevX < BOARD_SIZE && prevY >= 0 && prevY < BOARD_SIZE) {
                if (board.at(static_cast<uint8_t>(prevX), static_cast<uint8_t>(prevY)) == Cell::Empty)
                    ++openEnds;
            } else {
                // Off-board is closed
            }
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
                if (board.at(static_cast<uint8_t>(nx), static_cast<uint8_t>(ny)) == Cell::Empty)
                    ++openEnds;
            } else {
                // Off-board is closed
            }

            const int val = runValue(len, openEnds);
            if (c == me)
                patternScore += val;
            else if (c == opp)
                patternScore -= val;
        }
    }
    score += patternScore;

    return score;
}

// Ordonne les coups à explorer (Gomoku):
//  - 1) Coups gagnants immédiats (faire 5, capture gagnante),
//  - 2) Parades immédiates (bloquer 5 adverse, empêcher capture gagnante),
//  - 3) Création de quatre ouverts / blocage des quatre adverses,
//  - 4) Captures de paires critiques,
//  - 5) Extensions de menaces (étendre 3→4, 4→5) près du front,
//  - 6) Heuristique géométrique (proximité des pierres existantes), killers/history en option.
// TODO: placer ttMove en tête si dispo, puis classer par criticité des menaces/captures; plafonner à N coups pour maitriser le branching.
std::vector<Move> MinimaxSearch::orderMoves(const Board& board,
    const RuleSet& rules,
    Player toMove,
    const std::optional<Move>& ttMove) const
{
    // TODO: Placer ttMove en tête, réordonner par menaces/captures Gomoku, limiter à un top-N.
    (void)ttMove;
    return orderedMovesPublic(board, rules, toMove);
}

// Renvoie true si le temps est écoulé ou nodeCap atteint (soft stop).
// Note: nodeCap doit être incrémenté côté recherche (negamax/qsearch) pour être effectif.
bool MinimaxSearch::cutoffByTime(const SearchContext& ctx) const
{
    // TODO: Ajouter nodeCap check si besoin.
    return std::chrono::steady_clock::now() >= ctx.deadline;
}

// Détecte si la position est terminale (Gomoku): victoire (5 alignés ou par captures) ou nul.
// Score retourné: négatif au trait si l’adversaire vient de gagner (correction distance-mate incluse).
bool MinimaxSearch::isTerminal(const Board& board, int ply, int& outScore) const
{
    const auto st = board.status();
    switch (st) {
    case GameStatus::Ongoing:
        outScore = 0;
        return false;
    case GameStatus::WinByAlign:
    case GameStatus::WinByCapture:
        // Side to move has just been given a losing position (opponent won on previous move)
        outScore = -MATE_SCORE + ply; // mate distance correction
        return true;
    case GameStatus::Draw:
        outScore = 0;
        return true;
    }
    outScore = 0;
    return false;
}

// Interroge la TT: si une entrée suffisante existe, fournit un bound et/ou un coup d’appoint.
// TODO (plus tard): clé zobrist, profondeur, flags (Exact/Lower/Upper), meilleur coup pour l’ordre.
bool MinimaxSearch::ttProbe(const Board& board, int depth, int alpha, int beta, int& outScore, std::optional<Move>& ttMove, TranspositionTable::Flag& outFlag) const
{
    // TODO: Chercher dans tt, vérifier profondeur, renvoyer score/flag/move si applicable.
    (void)board;
    (void)depth;
    (void)alpha;
    (void)beta;
    (void)outScore;
    (void)ttMove;
    (void)outFlag;
    return false;
}

// Stocke un résultat dans la TT (clé, profondeur, score, flag, meilleur coup).
// TODO (plus tard): politique de remplacement (plus profond/plus récent), correction des scores de mate pour l’indexation.
void MinimaxSearch::ttStore(const Board& board, int depth, int score, TranspositionTable::Flag flag, const std::optional<Move>& best)
{
    // TODO: Calculer zobrist, remplir l'entrée, politique de remplacement.
    (void)board;
    (void)depth;
    (void)score;
    (void)flag;
    (void)best;
}

// --- Helpers extracted from bestMove ---
// Raccourci “gain immédiat” (Gomoku):
//  - Ne teste que si plausible: ≥4 pierres posées (alignement possible en 1) ou ≥4 paires capturées (capture-win possible).
//  - Joue spéculativement chaque candidat; si status devient WinByAlign/WinByCapture, retourne ce coup immédiatement.
//  - Laisse les règles gérer les interdits (double-trois, overline (6+)) via tryPlay/status.
std::optional<Move> MinimaxSearch::tryImmediateWinShortcut(Board& board, const RuleSet& rules, Player toPlay, const std::vector<Move>& candidates) const
{

    const bool plausibleAlign = board.stoneCount(toPlay) >= 4;
    const auto caps = board.capturedPairs();
    const int pairs = (toPlay == Player::Black) ? caps.black : caps.white;
    const bool plausibleCaptureWin = pairs >= 4;

    // Only attempt if at least one path to immediate win is plausible
    if (!plausibleAlign && !plausibleCaptureWin)
        return std::nullopt;

    for (const auto& m : candidates) {
        auto pr = board.tryPlay(m, rules);
        if (!pr.success)
            continue; // skip illegal candidates, don't abort early
        const auto st = board.status();
        board.undo();
        if (st == GameStatus::WinByAlign || st == GameStatus::WinByCapture)
            return m;
    }
    return std::nullopt;
}

bool MinimaxSearch::runDepth(int depth, Board& board, const RuleSet& rules, Player toPlay, const std::vector<Move>& rootCandidates, std::optional<Move>& best, int& bestScore, std::vector<Move>& pv, long long& nodes, const SearchContext& ctx)
{
    if (cutoffByTime(ctx) || Clock::now() >= ctx.deadline)
        return false;

    std::optional<Move> ttRootMove;
    int ttScore = 0;
    TranspositionTable::Flag ttFlag = TranspositionTable::Flag::Exact;
    (void)ttProbe(board, depth, -INF, INF, ttScore, ttRootMove, ttFlag);

    auto ordered = orderMoves(board, rules, toPlay, ttRootMove);
    if (ordered.empty())
        ordered = rootCandidates; // fallback

    int alpha = -INF, beta = INF;
    std::optional<Move> depthBest;
    int depthBestScore = -INF;
    std::vector<Move> depthPV;

    for (const auto& m : ordered) {
        if (cutoffByTime(ctx) || Clock::now() >= ctx.deadline)
            break;
        auto pr = board.tryPlay(m, rules);
        if (!pr.success)
            continue;
        std::vector<Move> childPV;
        int childScore = negamax(board, depth - 1, -beta, -alpha, /*ply*/ 1, childPV, ctx);
        int score = -childScore;
        ++nodes;
        board.undo();

        if (score > depthBestScore) {
            depthBestScore = score;
            depthBest = m;
            depthPV.clear();
            depthPV.push_back(m);
            depthPV.insert(depthPV.end(), childPV.begin(), childPV.end());
        }
        if (score > alpha)
            alpha = score;
    }

    if (!depthBest)
        return false;

    best = depthBest;
    bestScore = depthBestScore;
    pv = depthPV;
    ttStore(board, depth, bestScore, TranspositionTable::Flag::Exact, best);
    return true;
}
} // namespace gomoku
