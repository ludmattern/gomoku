#pragma once
#include "gomoku/Types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

class ABoardView {
public:
    virtual ~ABoardView() = default;

    // Core access methods
    virtual Cell at(uint8_t x, uint8_t y) const = 0;
    virtual Cell at(const Pos& pos) const { return at(pos.x, pos.y); }

    // Game state
    virtual Player toPlay() const = 0;
    virtual CaptureCount capturedPairs() const = 0;
    virtual GameStatus status() const = 0;

    // Board analysis
    virtual bool isBoardFull() const = 0;
    virtual std::vector<Move> legalMoves(Player p, const RuleSet& rules) const = 0;

    // Hash for transposition tables
    virtual uint64_t zobristKey() const = 0;
};

} // namespace gomoku
