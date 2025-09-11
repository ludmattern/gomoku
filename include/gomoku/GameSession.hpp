#pragma once
#include "gomoku/Engine.hpp" // expose ABoardView, Engine, RuleSet, Move
#include "gomoku/SearchStats.hpp"
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

struct GameSnapshot {
    const ABoardView* view; // pointeur stable vers l’état courant
    std::optional<Pos> lastMove; // dernier coup joué (si connu)
    Player toPlay;
    std::pair<int, int> captures; // (●, ○)
    GameStatus status;
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

class GameSession {
public:
    explicit GameSession(const RuleSet& rules = RuleSet {}, Controller black = Controller::Human, Controller white = Controller::AI);

    // Snapshot lisible par toute UI
    GameSnapshot snapshot() const;

    // Contrôleurs
    void setController(Player side, Controller c);
    Controller controller(Player side) const;

    // Coups
    GamePlayResult playHuman(Pos p); // vérifie isLegal + play
    GamePlayResult playAI(int timeMs = 450); // bestMove + play
    bool undo(int halfMoves = 1); // 1=dernier coup, 2=un “tour”
    void reset(Player start = Player::Black);

    // Utilitaires
    std::optional<Move> hint(int timeMs, SearchStats* outStats = nullptr) const;

    // Accès bas-niveau si besoin
    const Engine& engine() const { return eng_; }
    Engine& engine() { return eng_; }

private:
    RuleSet rules_;
    Engine eng_;
    std::optional<Pos> last_;
    Controller black_ = Controller::Human;
    Controller white_ = Controller::AI;

    Controller ctrl(Player p) const { return (p == Player::Black ? black_ : white_); }
};

} // namespace gomoku
