#include "gomoku/infrastructure/MemoryBoardRepository.hpp"

namespace gomoku::infrastructure {

bool MemoryBoardRepository::save(const std::string& id, const GameState& state)
{
    store_[id] = state; // copy assignment
    return true;
}

std::optional<GameState> MemoryBoardRepository::load(const std::string& id)
{
    auto it = store_.find(id);
    if (it == store_.end())
        return std::nullopt;
    return it->second; // copy
}

} // namespace gomoku::infrastructure
