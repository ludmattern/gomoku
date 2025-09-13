#pragma once
#include "gomoku/ai/MinimaxSearch.hpp"
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/ISearchEngine.hpp"

namespace gomoku::ai {

/**
 * Concrete implementation of ISearchEngine using minimax search
 *
 * Wraps the existing MinimaxSearch class to implement the ISearchEngine interface,
 * providing clean separation between AI concerns and game orchestration.
 */
class MinimaxSearchEngine : public ISearchEngine {
public:
    explicit MinimaxSearchEngine(const SearchConfig& config = {});
    ~MinimaxSearchEngine() override = default;

    // Configuration methods
    void setTimeLimit(int milliseconds) override;
    void setDepthLimit(int maxDepth) override;
    void setTranspositionTableSize(size_t bytes) override;

    // MinimaxSearch operations
    std::optional<Move> findBestMove(
        const IBoardView& board,
        const RuleSet& rules,
        SearchStats* stats = nullptr) override;

    std::optional<Move> suggestMove(
        const IBoardView& board,
        const RuleSet& rules,
        int timeMs,
        SearchStats* stats = nullptr) override;

    // Analysis methods
    int evaluatePosition(const IBoardView& board, Player perspective) const override;
    std::vector<Move> getOrderedMoves(const IBoardView& board, const RuleSet& rules) const override;

    // Statistics and debugging
    void clearTranspositionTable() override;
    SearchStats getLastSearchStats() const override;

private:
    MinimaxSearch searchImpl_;
    SearchConfig config_;
    SearchStats lastStats_;

    // Helper to convert IBoardView to concrete Board for MinimaxSearch class
    class Board boardFromView(const IBoardView& view) const;
};

} // namespace gomoku::ai
