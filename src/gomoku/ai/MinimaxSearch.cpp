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

// Négamax récursif avec alpha-bêta, PVS, TT, extensions, etc.
// Explore les coups enfants, applique l'évaluation statique ou qsearch en feuille, remplit la PV.
int MinimaxSearch::negamax(Board& board,
    int depth,
    int alpha,
    int beta,
    int ply,
    std::vector<Move>& pvOut,
    const SearchContext& ctx)
{
    // TODO: Terminal check, TT probe, qsearch, boucle sur orderMoves, tryPlay/undo, appel récursif, alpha/beta update, TT store, PV build.
    (void)board;
    (void)depth;
    (void)alpha;
    (void)beta;
    (void)ply;
    (void)ctx;
    pvOut.clear();
    return 0;
}

// Recherche de quiétude : explore uniquement les coups tactiques (captures, menaces), stabilise l'évaluation.
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

// Évaluation statique rapide d'une position (patterns, captures, centralité, etc.).
int MinimaxSearch::evaluate(const Board& board, Player perspective) const
{
    // TODO: Détection 5 alignés, captures, menaces, heuristique géométrique, etc.
    (void)board;
    (void)perspective;
    return 0;
}

// Ordonne les coups à explorer : ttMove, tactiques, killers, history, heuristique géométrique.
std::vector<Move> MinimaxSearch::orderMoves(const Board& board,
    const RuleSet& rules,
    Player toMove,
    const std::optional<Move>& ttMove) const
{
    // TODO: Placer ttMove en tête, puis coups tactiques, puis tri heuristique.
    (void)ttMove;
    return orderedMovesPublic(board, rules, toMove);
}

// Renvoie true si le temps est écoulé ou nodeCap atteint (soft stop).
bool MinimaxSearch::cutoffByTime(const SearchContext& ctx) const
{
    // TODO: Ajouter nodeCap check si besoin.
    return std::chrono::steady_clock::now() >= ctx.deadline;
}

// Détecte si la position est terminale (victoire, nul, etc.) et renvoie le score associé.
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

// Interroge la table de transposition : si une entrée profonde existe, fournit un bound ou un coup.
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
