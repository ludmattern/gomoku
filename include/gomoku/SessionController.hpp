#pragma once
// Reworked SessionController: now a thin UI/session adapter over GameService
// (previously wrapped Engine directly). Engine responsibilities already
// encapsulate GameService; to reduce duplication we depend directly on
// GameService which owns the Board and AI search engine.

#include "gomoku/ai/SearchStats.hpp"
#include "gomoku/application/GameService.hpp"
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IBoardView.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

struct GameSnapshot {
    const IBoardView* view; // Current board view
    std::optional<Pos> lastMove; // Last move played if any
    Player toPlay; // Side to play
    std::pair<int, int> captures; // (black, white) captured pairs
    GameStatus status; // Game status
};

enum class Controller {
    Human,
    AI
};

struct GamePlayResult {
    bool ok;
    std::string why; // si !ok
    std::optional<Move> mv; // coup effectivement joué
    std::optional<SearchStats> stats; // rempli pour IA
};

class SessionController {
public:
    explicit SessionController(const RuleSet& rules = RuleSet {}, Controller black = Controller::Human, Controller white = Controller::AI);

    GameSnapshot snapshot() const;

    // Controller configuration
    void setController(Player side, Controller c);
    Controller controller(Player side) const;

    // Moves
    GamePlayResult playHuman(Pos p); // validate + play
    GamePlayResult playAI(int timeMs = 450); // search + play
    bool undo(int halfMoves = 1); // undo half-moves
    void reset(Player start = Player::Black);

    // Utilities
    std::optional<Move> hint(int timeMs, SearchStats* outStats = nullptr) const;

    // Expose underlying board view (read-only)
    const IBoardView& board() const { return gameService_->getBoard(); }

private:
    RuleSet rules_;
    std::unique_ptr<application::GameService> gameService_;
    std::optional<Pos> last_;
    Controller black_ = Controller::Human;
    Controller white_ = Controller::AI;

    Controller ctrl(Player p) const { return (p == Player::Black ? black_ : white_); }
};

} // namespace gomoku
