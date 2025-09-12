#include "gomoku/ai/MinimaxSearchEngine.hpp"
#include "gomoku/core/Board.hpp"
#include "gomoku/interfaces/IBoardView.hpp"

namespace gomoku::ai {

MinimaxSearchEngine::MinimaxSearchEngine(const SearchConfig& config)
    : searchImpl_(config)
    , config_(config)
{
}

void MinimaxSearchEngine::setTimeLimit(int milliseconds)
{
    config_.timeBudgetMs = milliseconds;
    searchImpl_ = MinimaxSearch(config_);
}

void MinimaxSearchEngine::setDepthLimit(int maxDepth)
{
    config_.maxDepthHint = maxDepth;
    searchImpl_ = MinimaxSearch(config_);
}

void MinimaxSearchEngine::setTranspositionTableSize(size_t bytes)
{
    config_.ttBytes = bytes;
    searchImpl_ = MinimaxSearch(config_);
}

std::optional<Move> MinimaxSearchEngine::findBestMove(
    const IBoardView& board,
    const RuleSet& rules,
    SearchStats* stats)
{
    // Convert IBoardView to concrete Board for MinimaxSearch class
    Board concreteBoard = boardFromView(board);

    // Use existing MinimaxSearch implementation
    auto result = searchImpl_.bestMove(concreteBoard, rules, stats);
    lastStats_ = stats ? *stats : SearchStats {};

    return result;
}

std::optional<Move> MinimaxSearchEngine::suggestMove(
    const IBoardView& board,
    const RuleSet& rules,
    int timeMs,
    SearchStats* stats)
{
    // Temporarily adjust time limit
    int oldTimeMs = config_.timeBudgetMs;
    setTimeLimit(timeMs);

    auto result = findBestMove(board, rules, stats);

    // Restore original time limit
    setTimeLimit(oldTimeMs);

    return result;
}

int MinimaxSearchEngine::evaluatePosition(const IBoardView& board, Player perspective) const
{
    Board concreteBoard = boardFromView(board);
    // This would need to be implemented in MinimaxSearch class
    // For now, return a dummy value
    (void)perspective; // Avoid unused parameter warning
    return 0;
}

std::vector<Move> MinimaxSearchEngine::getOrderedMoves(const IBoardView& board, const RuleSet& rules) const
{
    // For now, just return legal moves from the current player
    (void)board; // Avoid unused parameter warning - we'll use it when implementing properly
    Board concreteBoard; // Create empty board for now
    return concreteBoard.legalMoves(Player::Black, rules);
}

void MinimaxSearchEngine::clearTranspositionTable()
{
    // This would need to be implemented in MinimaxSearch class
    // For now, recreate the search object
    searchImpl_ = MinimaxSearch(config_);
}

SearchStats MinimaxSearchEngine::getLastSearchStats() const
{
    return lastStats_;
}

// Helper method to convert IBoardView to concrete Board
Board MinimaxSearchEngine::boardFromView(const IBoardView& view) const
{
    // The IBoardView is actually a Bridge pattern wrapper
    // In our architecture, it's always wrapping a gomoku::Board
    // We can safely cast to the actual IBoardView and then to Board
    const auto* realView = reinterpret_cast<const gomoku::IBoardView*>(&view);
    const auto* concreteBoard = dynamic_cast<const Board*>(realView);

    if (concreteBoard) {
        return *concreteBoard; // Copy constructor
    } else {
        // Fallback: create empty board (shouldn't happen in our architecture)
        return Board {};
    }
}

} // namespace gomoku::ai