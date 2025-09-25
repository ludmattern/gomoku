// gomoku/ai/CandidateGenerator.hpp
#pragma once
#include "gomoku/core/Board.hpp"
#include "gomoku/core/Logger.hpp"
#include "gomoku/core/Types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

struct CandidateConfig {
    uint8_t groupGap = 1; // distance Chebyshev pour grouper les îlots
    uint8_t margin = 2; // dilatation des rectangles
    uint8_t ringR = 2; // anneau de génération autour des pierres
    uint16_t maxCandidates = 64; // réduire le plafond pour limiter le facteur de branchement
    bool includeOpponentRing = true; // anneau aussi autour des pierres adverses
};

class CandidateGenerator {
public:
    static std::vector<Move> generate(const Board& b, const RuleSet& rules,
        Player toPlay, const CandidateConfig& cfg);
};

} // namespace gomoku
