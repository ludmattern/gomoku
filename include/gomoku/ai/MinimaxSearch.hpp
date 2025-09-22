#pragma once
#include "gomoku/ai/QuiescenceSearch.hpp"
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
    QuiescenceSearch::Config quiescence; // Configuration de la quiescence

    // Late Move Reduction (LMR) parameters
    bool lmrEnabled = true; // Activer/désactiver LMR
    int lmrMinDepth = 3; // Profondeur minimale pour appliquer LMR
    int lmrMoveThreshold = 4; // Indice de mouvement à partir duquel appliquer LMR
    int lmrReduction = 1; // Réduction de profondeur (peut être adaptative)
};

class MinimaxSearch {
public:
    explicit MinimaxSearch(const SearchConfig& conf)
        : cfg(conf)
        , quiescenceSearch_(conf.quiescence)
    {
        clearKillersAndHistory();
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
    void setMaxQuiescenceDepth(int depth) { cfg.quiescence.maxDepth = depth; }
    void setQuiescenceEnabled(bool enabled) { cfg.quiescence.enabled = enabled; }

    // Late Move Reduction configuration
    void setLMREnabled(bool enabled) { cfg.lmrEnabled = enabled; }
    void setLMRMinDepth(int depth) { cfg.lmrMinDepth = depth; }
    void setLMRMoveThreshold(int threshold) { cfg.lmrMoveThreshold = threshold; }
    void setLMRReduction(int reduction) { cfg.lmrReduction = reduction; }

    void clearTranspositionTable() { tt.resizeBytes(cfg.ttBytes); }
    void clearKillersAndHistoryPublic() { clearKillersAndHistory(); }

    int evaluatePublic(const Board& b, Player pov) const { return evaluate(b, pov); }
    std::vector<Move> orderedMovesPublic(Board& b, const RuleSet& rules, Player toPlay) const { return orderedMoves(b, rules, toPlay, 0); }

private:
    SearchConfig cfg {};

    // --- état runtime
    std::chrono::steady_clock::time_point t0;
    int budgetMs = 0;
    bool timeUp = false;

    mutable unsigned long long visited = 0;

    // --- Killer Moves & History Heuristic
    static constexpr int MAX_DEPTH = 64;
    static constexpr int MAX_KILLERS = 2; // 2 killer moves par profondeur
    Move killerMoves[MAX_DEPTH][MAX_KILLERS];

    // History Heuristic : [x][y] -> score d'historique
    int historyTable[15][15];

    TranspositionTable tt;
    QuiescenceSearch quiescenceSearch_;

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
        if (cfg.nodeCap > 0 && visited >= cfg.nodeCap)
            return true;
        return false;
    }

    // Génération + ordering (léger)
    std::vector<Move> orderedMoves(Board& b, const RuleSet& rules, Player toPlay, int depth = 0) const;
    int quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const;

    // Killer Moves & History Heuristic
    void clearKillersAndHistory();
    void storeKiller(int depth, const Move& move);
    bool isKillerMove(int depth, const Move& move) const;
    void updateHistory(const Move& move, int depth);
    int getHistoryScore(const Move& move) const;

    // Late Move Reduction
    int calculateLMRReduction(int depth, int moveIndex, bool isPVNode) const;
};

} // namespace gomoku
