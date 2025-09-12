#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IBoardView.hpp"
#include <iostream>

namespace gomoku {

std::ostream& operator<<(std::ostream& os, Player p)
{
    return os << (p == Player::Black ? "Black" : "White");
}

std::ostream& operator<<(std::ostream& os, Cell c)
{
    switch (c) {
    case Cell::Empty:
        return os << "Empty";
    case Cell::Black:
        return os << "Black";
    case Cell::White:
        return os << "White";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Pos& pos)
{
    return os << "(" << static_cast<unsigned>(pos.x) << "," << static_cast<unsigned>(pos.y) << ")";
}

std::ostream& operator<<(std::ostream& os, const Move& move)
{
    return os << move.by << " at " << move.pos;
}

// GameState implementation
GameState GameState::fromBoard(const IBoardView& boardView, const std::vector<Move>& history, const RuleSet& gameRules)
{
    GameState state;

    // Copy board state
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            state.board[y * BOARD_SIZE + x] = boardView.at(x, y);
        }
    }

    // Copy other properties
    state.moveHistory = history;
    state.currentPlayer = boardView.toPlay();
    state.captures = boardView.capturedPairs();
    state.status = boardView.status();
    state.rules = gameRules;

    return state;
}

} // namespace gomoku
