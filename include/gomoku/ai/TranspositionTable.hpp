#pragma once
#include "gomoku/core/Types.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace gomoku {

class TranspositionTable {
public:
    enum class Flag : uint8_t { Exact,
        Lower,
        Upper };
    struct Entry {
        uint64_t key = 0;
        int score = 0;
        int depth = -1;
        Flag flag = Flag::Exact;
        Move best {}; // stored best move (or default)
    };

    TranspositionTable() = default;

    void resizeBytes(std::size_t bytes)
    {
        if (!bytes)
            bytes = (16ull << 20);
        std::size_t n = bytes / sizeof(Entry);
        if (n < 1024)
            n = 1024;
        std::size_t pow2 = 1;
        while (pow2 < n)
            pow2 <<= 1;
        table.assign(pow2, Entry {});
        mask = pow2 - 1;
    }

    [[nodiscard]] Entry* probe(uint64_t key) const
    {
        if (table.empty())
            return nullptr;
        return const_cast<Entry*>(&table[key & mask]);
    }

    void store(uint64_t key, int depth, int score, Flag flag, const std::optional<Move>& best)
    {
        if (table.empty())
            return;
        auto& e = table[key & mask];
        if (e.key != key || depth >= e.depth) {
            e.key = key;
            e.depth = depth;
            e.score = score;
            e.flag = flag;
            e.best = best.value_or(Move { Pos { 0, 0 }, Player::Black });
        }
    }

private:
    std::vector<Entry> table;
    std::size_t mask = 0;
};

} // namespace gomoku
