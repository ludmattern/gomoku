#pragma once
#include "gomoku/application/MoveValidator.hpp"
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IGameObserver.hpp"
#include "gomoku/interfaces/IGameService.hpp"
#include "gomoku/interfaces/ISearchEngine.hpp"
#include <memory>
#include <vector>

// Forward declarations
namespace gomoku {
class Board;
}

namespace gomoku::application {

/**
 * Concrete implementation of IGameService
 *
 * Orchestrates game flow by coordinating Board, Rules, and AI components.
 * Follows Single Responsibility Principle - focuses only on game orchestration.
 */
class GameService : public IGameService {
public:
    explicit GameService(
        std::unique_ptr<ISearchEngine> searchEngine = nullptr);

    ~GameService() override; // Defined in .cpp to avoid incomplete type issues

    // IGameService implementation
    void startNewGame(const RuleSet& rules = {}) override;
    void reset() override;
    GameStatus getGameStatus() const override;
    Player getCurrentPlayer() const override;

    PlayResult makeMove(const Position& pos) override;
    PlayResult makeMove(const Move& move) override;
    bool canUndo() const override;
    bool undo() override;

    const IBoardView& getBoard() const override;
    std::vector<Move> getLegalMoves() const override;

    bool isMoveLegal(const Move& move, std::string* reason = nullptr) const override;

    const std::vector<Move>& getMoveHistory() const override { return moveHistory_; }
    CaptureCount getCaptureCount() const override;

    // Additional GameService specific methods
    void setRules(const RuleSet& rules) { rules_ = rules; }
    const RuleSet& getRules() const { return rules_; }

    // AI integration
    std::optional<Move> getAIMove(int timeMs = 450);
    void setSearchEngine(std::unique_ptr<ISearchEngine> engine);

    // Observer management
    void addObserver(IGameObserver* obs);
    void removeObserver(IGameObserver* obs);

private:
    // Core game state
    std::unique_ptr<::gomoku::Board> board_;
    RuleSet rules_;
    std::vector<Move> moveHistory_;

    // Dependencies (injected)
    std::unique_ptr<ISearchEngine> searchEngine_;
    MoveValidator moveValidator_;

    // Observers (non owning raw pointers; lifetime managed by caller)
    std::vector<IGameObserver*> observers_;

    // Notify helpers
    void notifyGameStarted();
    void notifyMovePlayed(const Move& move);
    void notifyUndo();
    void notifyGameEnded();

    // Internal helpers
    bool validateMove(const Move& move, std::string* reason) const;
};

} // namespace gomoku::application