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
    int maxQuiescenceDepth = 4; // Profondeur maximale pour la recherche de quiescence
    bool enableQuiescence = true; // Activer/désactiver la recherche de quiescence
};

class MinimaxSearch {
public:
    explicit MinimaxSearch(const SearchConfig& conf)
        : cfg(conf)
    {
    }

    std::optional<Move> bestMove(Board& board, const RuleSet& rules, SearchStats* stats);

    // --- Public utility / configuration API (added) ---
    void setTimeBudgetMs(int ms) { cfg.timeBudgetMs = ms; }
    void setMaxDepthHint(int d) { cfg.maxDepthHint = d; }
    void setTranspositionTableSize(std::size_t bytes)
    {
        cfg.ttBytes = bytes;
        tt.resizeBytes(bytes);
    }
    void setNodeCap(unsigned long long cap) { cfg.nodeCap = cap; }
    void setMaxQuiescenceDepth(int depth) { cfg.maxQuiescenceDepth = depth; }
    void setQuiescenceEnabled(bool enabled) { cfg.enableQuiescence = enabled; }

    void clearTranspositionTable() { tt.resizeBytes(cfg.ttBytes); }

    int evaluatePublic(const Board& b, Player pov) const { return evaluate(b, pov); }
    std::vector<Move> orderedMovesPublic(Board& b, const RuleSet& rules, Player toPlay) const { return orderedMoves(b, rules, toPlay); }

private:
    SearchConfig cfg {};

    // --- état runtime
    std::chrono::steady_clock::time_point t0;
    int budgetMs = 0;
    bool timeUp = false;

    mutable unsigned long long visited = 0;
    unsigned long long nodeCap = 0;

    TranspositionTable tt;

    int evaluate(const Board& b, Player pov) const;
    int evaluateOneDir(const Board& b, uint8_t x, uint8_t y, Cell who, int dx, int dy) const;

    struct ABResult {
        int score;
        std::optional<Move> move;
    };

    ABResult alphabeta(Board& b, const RuleSet& rules, int depth, int alpha, int beta, Player maxPlayer, SearchStats* stats);
    ABResult quiescence(Board& b, const RuleSet& rules, int alpha, int beta, Player maxPlayer, SearchStats* stats, int qDepth = 0);

    inline bool expired() const
    {
        using namespace std::chrono;
        if ((int)duration_cast<milliseconds>(steady_clock::now() - t0).count() >= budgetMs)
            return true;
        if (nodeCap > 0 && visited >= nodeCap)
            return true;
        return false;
    }

    // Génération + ordering (léger)
    std::vector<Move> orderedMoves(Board& b, const RuleSet& rules, Player toPlay) const;
    std::vector<Move> tacticalMoves(Board& b, const RuleSet& rules, Player toPlay) const;
    int quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const;
    bool isQuiet(const Board& b, const RuleSet& rules, Player toPlay) const;
    bool isTacticalMove(const Board& b, uint8_t x, uint8_t y, Player toPlay) const;
};

} // namespace gomoku
