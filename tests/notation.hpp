#pragma once
#include "gomoku/core/Types.hpp"
#include <string>

namespace gomoku::notation {

// Return column label like A, B, C... (0-based).
// We keep it simple: A..Z without skipping letters.
inline char colLabel(int x)
{
    if (x < 0)
        return '?';
    // Wrap after Z if BOARD_SIZE > 26
    int base = x % 26;
    return static_cast<char>('A' + base);
}

// Convert position to string like "A1", "K11" (optional helper)
inline std::string toString(Pos p)
{
    std::string s;
    s += colLabel(p.x);
    s += std::to_string(static_cast<int>(p.y) + 1);
    return s;
}

} // namespace gomoku::notation
