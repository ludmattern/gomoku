#include "gomoku/application/GameService.hpp"
#include "gomoku/core/Board.hpp"
#include <algorithm>
#include <stdexcept>

namespace gomoku::application {

GameService::GameService(
    std::unique_ptr<ISearchEngine> searchEngine)
    : board_(std::make_unique<Board>())
    , searchEngine_(std::move(searchEngine))
{
}

GameService::~GameService() = default;

void GameService::startNewGame(const RuleSet& rules)
{
    rules_ = rules;
    board_->reset();
    moveHistory_.clear();
}

void GameService::reset()
{
    board_->reset();
    moveHistory_.clear();
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
        PlayErrorCode code = PlayErrorCode::RuleViolation;
        if (reason == "Invalid position")
            code = PlayErrorCode::InvalidPosition;
        else if (reason == "Not this player's turn")
            code = PlayErrorCode::NotPlayersTurn;
        else if (reason == "Position already occupied")
            code = PlayErrorCode::Occupied;
        else if (reason == "Game already finished")
            code = PlayErrorCode::GameFinished;
        // Reste: RuleViolation (double-three, must-break...) déjà mappé
        return PlayResult::fail(code, reason);
    }

    // Attempt to play the move
    auto result = board_->tryPlay(move, rules_);
    if (result.success) {
        moveHistory_.push_back(move);
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


bool GameService::validateMove(const Move& move, std::string* reason) const
{
    auto base = moveValidator_.validate(*board_, rules_, move);
    if (!base.ok) {
        if (reason)
            *reason = base.reason;
        return false;
    }

    auto self = const_cast<GameService*>(this);
    PlayResult pr;
    bool ok = self->board_->speculativeTry(move, rules_, &pr);
    if (!ok) {
        if (reason)
            *reason = pr.error;
        return false;
    }
    return true;
}

} // namespace gomoku::application
