#pragma once
#include "gomoku/interfaces/IBoardRepository.hpp"
#include <memory>
#include <unordered_map>

namespace gomoku::infrastructure {

/**
 * @brief In-memory implementation of IBoardRepository
 *
 * Simple implementation that stores game states in memory.
 * Suitable for testing and development. For production,
 * consider FileRepository or DatabaseRepository.
 */
class MemoryBoardRepository : public IBoardRepository {
public:
    MemoryBoardRepository() = default;
    ~MemoryBoardRepository() override = default;

    // IBoardRepository implementation
    bool save(const std::string& gameId, const GameState& state) override;
    std::optional<GameState> load(const std::string& gameId) override;
    bool exists(const std::string& gameId) const override;
    bool remove(const std::string& gameId) override;

    std::vector<std::string> listSavedGames() const override;
    std::vector<GameMetadata> getGameMetadata() const override;

    void clear() override;
    size_t getStorageSize() const override;

private:
    std::unordered_map<std::string, GameState> gameStates_;
    std::unordered_map<std::string, GameMetadata> metadata_;

    // Helper to create metadata from game state
    GameMetadata createMetadata(const std::string& gameId, const GameState& state);
};

} // namespace gomoku::infrastructure