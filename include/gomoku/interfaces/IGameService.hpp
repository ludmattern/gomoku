#pragma once
#include "IBoardView.hpp"
#include "gomoku/core/Types.hpp"
#include <optional>
#include <string>

namespace gomoku {

/**
 * Interface for game orchestration and business logic
 *
 * This interface defines the contract for game management,
 * separating business logic from implementation details.
 */
class IGameService {
public:
    virtual ~IGameService() = default;

    // Game state management
    virtual void startNewGame(const RuleSet& rules = {}) = 0;
    virtual void reset() = 0;
    virtual GameStatus getGameStatus() const = 0;
    virtual Player getCurrentPlayer() const = 0;

    // Move operations
    virtual PlayResult makeMove(const Position& pos) = 0;
    virtual PlayResult makeMove(const Move& move) = 0;
    virtual bool canUndo() const = 0;
    virtual bool undo() = 0;

    // Board access
    virtual const IBoardView& getBoard() const = 0;
    virtual std::vector<Move> getLegalMoves() const = 0;

    // Game validation
    virtual bool isMoveLegal(const Move& move, std::string* reason = nullptr) const = 0;

    // Game history and statistics
    virtual const std::vector<Move>& getMoveHistory() const = 0;
    virtual CaptureCount getCaptureCount() const = 0;
};

} // namespace gomoku