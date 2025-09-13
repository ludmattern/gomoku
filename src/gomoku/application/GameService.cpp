#include "gomoku/application/GameService.hpp"
#include "gomoku/core/Board.hpp"
#include <algorithm>
#include <stdexcept>

namespace gomoku::application {

GameService::GameService(
    std::unique_ptr<ISearchEngine> searchEngine,
    std::unique_ptr<IBoardRepository> repository)
    : board_(std::make_unique<Board>())
    , searchEngine_(std::move(searchEngine))
    , repository_(std::move(repository))
{
}

GameService::~GameService() = default;

void GameService::startNewGame(const RuleSet& rules)
{
    rules_ = rules;
    board_->reset();
    moveHistory_.clear();
    updateGameStatus();
}

void GameService::reset()
{
    board_->reset();
    moveHistory_.clear();
    updateGameStatus();
}

GameStatus GameService::getGameStatus() const
{
    return board_->status();
}

Player GameService::getCurrentPlayer() const
{
    return board_->toPlay();
}

PlayResult GameService::makeMove(const Position& pos)
{
    Move move { pos, getCurrentPlayer() };
    return makeMove(move);
}

PlayResult GameService::makeMove(const Move& move)
{
    // Validate the move first
    std::string reason;
    if (!isMoveLegal(move, &reason)) {
        return PlayResult::fail(reason);
    }

    // Attempt to play the move
    auto result = board_->tryPlay(move, rules_);
    if (result.success) {
        moveHistory_.push_back(move);
        updateGameStatus();
    }

    return result;
}

bool GameService::canUndo() const
{
    return !moveHistory_.empty();
}

bool GameService::undo()
{
    if (!canUndo()) {
        return false;
    }

    bool success = board_->undo();
    if (success && !moveHistory_.empty()) {
        moveHistory_.pop_back();
        updateGameStatus();
    }
    return success;
}

const IBoardView& GameService::getBoard() const
{
    return *board_;
}

std::vector<Move> GameService::getLegalMoves() const
{
    return board_->legalMoves(getCurrentPlayer(), rules_);
}

bool GameService::isMoveLegal(const Move& move, std::string* reason) const
{
    return validateMove(move, reason);
}

CaptureCount GameService::getCaptureCount() const
{
    return board_->capturedPairs();
}

std::optional<Move> GameService::getAIMove(int timeMs)
{
    if (!searchEngine_) {
        return std::nullopt;
    }

    SearchStats stats;
    auto move = searchEngine_->suggestMove(*board_, rules_, timeMs, &stats);

    // Store stats for debugging/analysis
    // Could be exposed through a getLastAIStats() method

    return move;
}

void GameService::setSearchEngine(std::unique_ptr<ISearchEngine> engine)
{
    searchEngine_ = std::move(engine);
}

bool GameService::saveGame(const std::string& gameId)
{
    if (!repository_) {
        return false;
    }

    GameState state = GameState::fromBoard(*board_, moveHistory_, rules_);
    return repository_->save(gameId, state);
}

bool GameService::loadGame(const std::string& gameId)
{
    if (!repository_) {
        return false;
    }

    auto stateOpt = repository_->load(gameId);
    if (!stateOpt) {
        return false;
    }

    const auto& state = *stateOpt;

    // Restore game state
    rules_ = state.rules;
    moveHistory_ = state.moveHistory;

    // Reset board and replay moves
    board_->reset();
    for (const auto& move : state.moveHistory) {
        auto result = board_->tryPlay(move, rules_);
        if (!result.success) {
            // Corrupted save file - reset to clean state
            board_->reset();
            moveHistory_.clear();
            return false;
        }
    }

    updateGameStatus();
    return true;
}

// Private methods
void GameService::updateGameStatus()
{
    // The board already tracks game status internally
    // This could be extended to trigger events, logging, etc.
}

bool GameService::validateMove(const Move& move, std::string* reason) const
{
    // Check basic validity
    if (!move.isValid()) {
        if (reason)
            *reason = "Invalid position";
        return false;
    }

    // Check if it's the correct player's turn
    if (move.by != getCurrentPlayer()) {
        if (reason)
            *reason = "Not this player's turn";
        return false;
    }

    // Check if position is empty
    if (board_->at(move.pos.x, move.pos.y) != Cell::Empty) {
        if (reason)
            *reason = "Position already occupied";
        return false;
    }

    // For basic gameplay, we can skip the restrictive windowing
    // and only check for pattern rules (double-three, etc.)
    // Create a temporary board to test the move
    Board tempBoard = *board_;
    auto result = tempBoard.tryPlay(move, rules_);

    if (!result.success && reason) {
        *reason = result.error;
    }

    return result.success;
}

} // namespace gomoku::application
