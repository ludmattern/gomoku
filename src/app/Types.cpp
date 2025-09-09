#include "gomoku/Types.hpp"
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

} // namespace gomoku
