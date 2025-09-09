#pragma once
#include "Types.hpp"
#include "gomoku/BoardView.hpp"
#include "gomoku/SearchStats.hpp"
#include <memory>
#include <string>
#include <vector>

namespace gomoku {

class Engine {
public:
    explicit Engine(const EngineConfig& cfg = {});
    ~Engine();

    const BoardView& board() const;

    // Partie
    void reset();
    bool isLegal(Move m, std::string* whyNot = nullptr) const;
    bool play(Move m); // applique un coup (si légal)
    bool undo();

    // Gestion du trait explicitement (utile pour GameSession::reset)
    void forceSide(Player p);

    // IA (implémentation minimale; brancher Minimax dans src/ai)
    std::optional<Move> bestMove(int timeBudgetMs, SearchStats* outStats = nullptr) const;
    std::optional<Move> suggest(int timeBudgetMs, SearchStats* outStats = nullptr) const
    {
        return bestMove(timeBudgetMs, outStats);
    }

    std::vector<Move> legalMoves() const;

private:
    struct Impl;
    std::unique_ptr<Impl> self;
};

} // namespace gomoku
