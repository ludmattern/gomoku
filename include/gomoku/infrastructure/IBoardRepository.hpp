#pragma once

#include "gomoku/core/Types.hpp"
#include <optional>
#include <string>

namespace gomoku::infrastructure {

/**
 * @brief Abstraction for persisting / restoring full game states.
 *
 * Current use is minimal (in-memory implementation) but this interface allows
 * future extension to file, database, or remote backends without changing
 * application layer orchestration.
 */
class IBoardRepository {
public:
    virtual ~IBoardRepository() = default;

    /**
     * Persist a full game state under a logical identifier.
     * @return true on success.
     */
    virtual bool save(const std::string& id, const GameState& state) = 0;

    /**
     * Load a previously saved game state.
     * @return std::nullopt if not found / corrupted.
     */
    virtual std::optional<GameState> load(const std::string& id) = 0;
};

} // namespace gomoku::infrastructure
