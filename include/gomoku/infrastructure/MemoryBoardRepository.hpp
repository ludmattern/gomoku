#pragma once

#include "gomoku/infrastructure/IBoardRepository.hpp"
#include <unordered_map>

namespace gomoku::infrastructure {

/**
 * @brief Simple in-memory repository used for tests / runtime sessions.
 * Data lifetime matches process lifetime; no persistence to disk.
 */
class MemoryBoardRepository : public IBoardRepository {
public:
    bool save(const std::string& id, const GameState& state) override;
    std::optional<GameState> load(const std::string& id) override;

private:
    std::unordered_map<std::string, GameState> store_;
};

} // namespace gomoku::infrastructure
