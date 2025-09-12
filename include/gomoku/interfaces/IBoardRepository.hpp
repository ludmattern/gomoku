#pragma once
#include "gomoku/core/Types.hpp"
#include <ctime>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

/**
 * @brief Metadata for saved games
 */
struct GameMetadata {
    std::string gameId;
    std::string playerBlack = "Human";
    std::string playerWhite = "AI";
    GameStatus status = GameStatus::Ongoing;
    int moveCount = 0;
    std::time_t saveTime;
    RuleSet rules;
};

/**
 * @brief Repository interface for board/game persistence
 *
 * Abstracts storage mechanisms (memory, file, database)
 * following the Repository pattern.
 */
class IBoardRepository {
public:
    virtual ~IBoardRepository() = default;

    // Persistence operations
    virtual bool save(const std::string& gameId, const GameState& state) = 0;
    virtual std::optional<GameState> load(const std::string& gameId) = 0;
    virtual bool exists(const std::string& gameId) const = 0;
    virtual bool remove(const std::string& gameId) = 0;

    // Listing and querying
    virtual std::vector<std::string> listSavedGames() const = 0;
    virtual std::vector<GameMetadata> getGameMetadata() const = 0;

    // Cleanup
    virtual void clear() = 0;
    virtual size_t getStorageSize() const = 0;
};

} // namespace gomoku