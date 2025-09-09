#pragma once
#include "gomoku/Types.hpp"
#include <vector>

namespace gomoku {

/// @brief Represents the statistics for a search
struct SearchStats {
    long long nodes = 0;
    long long qnodes = 0;
    int depthReached = 0;
    int timeMs = 0;
    int ttHits = 0;
    std::vector<Move> principalVariation;
};

} // namespace gomoku
