#include "gomoku/infrastructure/MemoryBoardRepository.hpp"
#include <algorithm>
#include <ctime>

namespace gomoku::infrastructure {

bool MemoryBoardRepository::save(const std::string& gameId, const GameState& state)
{
    gameStates_[gameId] = state;
    metadata_[gameId] = createMetadata(gameId, state);
    return true;
}

std::optional<GameState> MemoryBoardRepository::load(const std::string& gameId)
{
    auto it = gameStates_.find(gameId);
    if (it != gameStates_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool MemoryBoardRepository::exists(const std::string& gameId) const
{
    return gameStates_.find(gameId) != gameStates_.end();
}

bool MemoryBoardRepository::remove(const std::string& gameId)
{
    auto gameIt = gameStates_.find(gameId);
    auto metaIt = metadata_.find(gameId);

    bool removed = false;
    if (gameIt != gameStates_.end()) {
        gameStates_.erase(gameIt);
        removed = true;
    }
    if (metaIt != metadata_.end()) {
        metadata_.erase(metaIt);
        removed = true;
    }

    return removed;
}

std::vector<std::string> MemoryBoardRepository::listSavedGames() const
{
    std::vector<std::string> gameIds;
    gameIds.reserve(gameStates_.size());

    for (const auto& pair : gameStates_) {
        gameIds.push_back(pair.first);
    }

    return gameIds;
}

std::vector<GameMetadata> MemoryBoardRepository::getGameMetadata() const
{
    std::vector<GameMetadata> metadataList;
    metadataList.reserve(metadata_.size());

    for (const auto& pair : metadata_) {
        metadataList.push_back(pair.second);
    }

    // Sort by save time (most recent first)
    std::sort(metadataList.begin(), metadataList.end(),
        [](const GameMetadata& a, const GameMetadata& b) {
            return a.saveTime > b.saveTime;
        });

    return metadataList;
}

void MemoryBoardRepository::clear()
{
    gameStates_.clear();
    metadata_.clear();
}

size_t MemoryBoardRepository::getStorageSize() const
{
    // Rough estimation of memory usage
    size_t size = 0;

    // Game states
    size += gameStates_.size() * sizeof(GameState);

    // Metadata
    for (const auto& pair : metadata_) {
        size += pair.first.size() + sizeof(GameMetadata);
        size += pair.second.playerBlack.size() + pair.second.playerWhite.size();
    }

    return size;
}

GameMetadata MemoryBoardRepository::createMetadata(const std::string& gameId, const GameState& state)
{
    GameMetadata metadata;

    metadata.gameId = gameId;
    metadata.status = state.status;
    metadata.moveCount = static_cast<int>(state.moveHistory.size());
    metadata.saveTime = std::time(nullptr);
    metadata.rules = state.rules;

    // Default player names - could be extended to track actual player types
    metadata.playerBlack = "Player";
    metadata.playerWhite = "AI";

    return metadata;
}

} // namespace gomoku::infrastructure