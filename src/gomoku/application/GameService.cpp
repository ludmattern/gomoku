#include "gomoku/application/GameService.hpp"
#include "gomoku/core/Board.hpp"
#include "gomoku/infrastructure/IBoardRepository.hpp"
#include <algorithm>
#include <stdexcept>

namespace gomoku::application {

GameService::GameService(
    std::unique_ptr<ISearchEngine> searchEngine,
    std::unique_ptr<infrastructure::IBoardRepository> repository)
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
    notifyGameStarted();
}

void GameService::reset()
{
    board_->reset();
    moveHistory_.clear();
    updateGameStatus();
    notifyGameStarted(); // reset is observed as a (re)start
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
        updateGameStatus();
        notifyMovePlayed(move);
        if (getGameStatus() != GameStatus::Ongoing) {
            notifyGameEnded();
        }
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
        notifyUndo();
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

// ---- Observer management ----
void GameService::addObserver(IGameObserver* obs)
{
    if (!obs)
        return;
    if (std::find(observers_.begin(), observers_.end(), obs) == observers_.end()) {
        observers_.push_back(obs);
    }
}

void GameService::removeObserver(IGameObserver* obs)
{
    observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
}

void GameService::notifyGameStarted()
{
    for (auto* o : observers_) {
        o->onGameStarted(rules_, *board_);
    }
}

void GameService::notifyMovePlayed(const Move& move)
{
    for (auto* o : observers_) {
        o->onMovePlayed(move, *board_, getGameStatus());
    }
}

void GameService::notifyUndo()
{
    for (auto* o : observers_) {
        o->onUndo(*board_, getGameStatus());
    }
}

void GameService::notifyGameEnded()
{
    for (auto* o : observers_) {
        o->onGameEnded(getGameStatus(), *board_);
    }
}

bool GameService::validateMove(const Move& move, std::string* reason) const
{
    auto base = moveValidator_.validate(*board_, rules_, move);
    if (!base.ok) {
        if (reason)
            *reason = base.reason;
        return false;
    }
    // Vérification profonde via simulation pour règles complexes
    Board tempBoard = *board_;
    auto result = tempBoard.tryPlay(move, rules_);
    if (!result.success) {
        if (reason)
            *reason = result.error;
        return false;
    }
    return true;
}

} // namespace gomoku::application