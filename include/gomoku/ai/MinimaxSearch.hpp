#pragma once
#include "gomoku/ai/SearchStats.hpp"
#include "gomoku/ai/TranspositionTable.hpp"
#include "gomoku/core/Types.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace gomoku {
class Board;

struct SearchConfig {
    int timeBudgetMs = 450; // Budget temps (ms) pour la recherche
    int maxDepthHint = 11; // Profondeur max d'itération
    std::size_t ttBytes = (64ull << 20); // Taille allouée à la table de transposition
    unsigned long long nodeCap = 0; // Limite de nœuds dure (0 = désactivée)
};

class MinimaxSearch {
public:
    explicit MinimaxSearch(const SearchConfig& conf)
        : cfg(conf)
    {
    }

    std::optional<Move> bestMove(Board& board, const RuleSet& rules, SearchStats* stats);

    // Configuration helpers used by MinimaxSearchEngine
    void setTimeBudgetMs(int ms) { cfg.timeBudgetMs = ms; }
    void setMaxDepthHint(int d) { cfg.maxDepthHint = d; }

    void setTranspositionTableSize(std::size_t bytes)
    {
        cfg.ttBytes = bytes;
        tt.resizeBytes(bytes);
    }

    void clearTranspositionTable() { tt.resizeBytes(cfg.ttBytes); }

    // Lightweight public helpers for tooling/analysis
    int evaluatePublic(const Board& board, Player perspective) const { return evaluate(board, perspective); }
    std::vector<Move> orderedMovesPublic(const Board& board, const RuleSet& rules, Player toPlay) const;

private:
    // Shared search context to avoid long parameter lists
    struct SearchContext {
        const RuleSet& rules;
        std::chrono::steady_clock::time_point deadline;
        SearchStats* stats { nullptr };
        unsigned long long nodeCap { 0 };
    };
    // --- Constants for search scoring ---
    static constexpr int INF = 1'000'000; // Generic infinity bound for alpha-beta
    static constexpr int MATE_SCORE = 900'000; // Base score for mate-like terminal outcomes

    // --- Core search primitives (signatures only) ---

    // Negamax with alpha-beta pruning and PVS. Returns best score and fills PV.
    // - depth: remaining plies to search (>= 0)
    // - alpha/beta: current search window
    // - toMove: side to move at this node
    // - ply: distance from root (for mate distance correction)
    // - stats: optional collector for node/qnode counters
    // - pvOut: principal variation to be populated with best line
    // - deadline: stop time for time management
    int negamax(Board& board,
        int depth,
        int alpha,
        int beta,
        int ply,
        std::vector<Move>& pvOut,
        const SearchContext& ctx);

    // Quiescence search to stabilize evaluations in tactical positions.
    // Searches only tactical moves (captures/menaces fortes) until a quiet position.
    int qsearch(Board& board,
        int alpha,
        int beta,
        int ply,
        const SearchContext& ctx);

    // Fast static evaluation of a position from a given perspective (side-to-move in negamax).
    int evaluate(const Board& board, Player perspective) const;

    // Move ordering at a node: combines TT move, tactical generator, killers/history, etc.
    // For now the implementation will reuse CandidateGenerator as a base and sort.
    std::vector<Move> orderMoves(const Board& board,
        const RuleSet& rules,
        Player toMove,
        const std::optional<Move>& ttMove) const;

    // Time management: returns true when we should abort the current search (soft stop).
    bool cutoffByTime(const SearchContext& ctx) const;

    // Terminal detection with score. Returns true if the position is terminal and sets outScore.
    bool isTerminal(const Board& board, int ply, int& outScore) const;

    // Transposition table helpers.
    // Attempt to read an entry and, if applicable, return a bound for cutoff.
    bool ttProbe(const Board& board, int depth, int alpha, int beta, int& outScore, std::optional<Move>& ttMove, TranspositionTable::Flag& outFlag) const;
    // Store a result into the TT.
    void ttStore(const Board& board, int depth, int score, TranspositionTable::Flag flag, const std::optional<Move>& best);

    // Lightweight accounting for stats (node/qnode incrementers, killer/history updates, etc.).
    inline void onNodeVisited(SearchStats* stats) const
    {
        if (stats)
            ++stats->nodes;
    }
    inline void onQNodeVisited(SearchStats* stats) const
    {
        if (stats)
            ++stats->qnodes;
    }

    // --- Helpers extracted from bestMove for readability ---
    // Tries the immediate win shortcut if plausible; returns the winning move if found.
    std::optional<Move> tryImmediateWinShortcut(Board& board,
        const RuleSet& rules,
        Player toPlay,
        const std::vector<Move>& candidates) const;

    // Runs one iterative-deepening step at a given depth; fills best, bestScore, pv and updates nodes.
    bool runDepth(int depth,
        Board& board,
        const RuleSet& rules,
        Player toPlay,
        const std::vector<Move>& rootCandidates,
        std::optional<Move>& best,
        int& bestScore,
        std::vector<Move>& pv,
        long long& nodes,
        const SearchContext& ctx);

    SearchConfig cfg {};
    TranspositionTable tt;
};

} // namespace gomoku
