#pragma once
#include "gomoku/ai/SearchStats.hpp"
#include "gomoku/core/Types.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace gomoku {
class Board;

// Moteur de recherche IA interne pour libgomoku_logic.a
// API publique : voir include/gomoku/Engine.hpp

struct SearchConfig {
    int timeBudgetMs = 450;
    int maxDepthHint = 6;
    std::size_t ttBytes = (64ull << 20); // mémoire pour la TT (défaut 64 MiB)
    unsigned long long nodeCap = 0; // 0 = illimité (contrôle strict par temps)
};

class MinimaxSearch {
public:
    explicit MinimaxSearch(const SearchConfig& cfg)
        : cfg(cfg)
    {
    }

    std::optional<Move> bestMove(Board& board, const RuleSet& rules, SearchStats* stats);

private:
    SearchConfig cfg {};

    // --- état runtime
    std::chrono::steady_clock::time_point t0;
    int budgetMs = 0;
    bool timeUp = false;

    // Coupe de sécurité par nœuds
    mutable unsigned long long visited = 0;
    unsigned long long nodeCap = 0;

    // --- TT ---
    enum class TTFlag : uint8_t {
        Exact,
        Lower,
        Upper
    };
    struct TTEntry {
        uint64_t key = 0;
        int score = 0;
        int depth = -1;
        TTFlag flag = TTFlag::Exact;
        Move best {};
    };
    std::vector<TTEntry> tt;
    std::size_t ttMask = 0;

    void initTT(std::size_t bytes);
    TTEntry* ttProbe(uint64_t key) const;
    void ttStore(uint64_t key, int depth, int score, TTFlag flag, const std::optional<Move>& best);

    // --- helpers
    int evaluate(const Board& b, Player pov) const;
    int evaluateOneDir(const Board& b, uint8_t x, uint8_t y, Cell who, int dx, int dy) const;

    struct ABResult {
        int score;
        std::optional<Move> move;
    };

    ABResult alphabeta(Board& b, const RuleSet& rules, int depth, int alpha, int beta, Player maxPlayer, SearchStats* stats);

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
    int quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const;

    static inline int manhattan(int x, int y, int cx, int cy)
    {
        return (x > cx ? x - cx : cx - x) + (y > cy ? y - cy : cy - y);
    }
};

} // namespace gomoku
