#pragma once
#include "gomoku/ai/SearchStats.hpp"
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IBoardView.hpp"
#include <optional>

namespace gomoku {

/**
 * @brief Interface for AI/MinimaxSearch engine capabilities
 *
 * Separates AI logic from game orchestration,
 * allowing different AI implementations to be plugged in.
 */
class ISearchEngine {
public:
    virtual ~ISearchEngine() = default;

    // Configuration
    virtual void setTimeLimit(int milliseconds) = 0;
    virtual void setDepthLimit(int maxDepth) = 0;
    virtual void setTranspositionTableSize(size_t bytes) = 0;

    // MinimaxSearch operations
    virtual std::optional<Move> findBestMove(
        const IBoardView& board,
        const RuleSet& rules,
        SearchStats* stats = nullptr)
        = 0;

    virtual std::optional<Move> suggestMove(
        const IBoardView& board,
        const RuleSet& rules,
        int timeMs,
        SearchStats* stats = nullptr)
        = 0;

    // Analysis
    virtual int evaluatePosition(const IBoardView& board, Player perspective) const = 0;
    virtual std::vector<Move> getOrderedMoves(const IBoardView& board, const RuleSet& rules) const = 0;

    // Statistics and debugging
    virtual void clearTranspositionTable() = 0;
    virtual SearchStats getLastSearchStats() const = 0;
};

} // namespace gomoku
