#pragma once
#include <array>
#include <cstdint>
#include <ctime>
#include <iosfwd>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gomoku {

/// @brief Board size (19x19 standard Gomoku)
inline constexpr int BOARD_SIZE = 19;

/// @brief Represents a player in the game
enum class Player : uint8_t {
    Black, // Black player (plays first)
    White // White player
};

/// @brief Represents the state of a cell on the board
enum class Cell : uint8_t {
    Empty, // Empty cell
    Black, // Cell occupied by black stone
    White // Cell occupied by white stone
};

// Utility functions for Player <-> Cell conversion
constexpr Cell playerToCell(Player p) noexcept
{
    return p == Player::Black ? Cell::Black : Cell::White;
}

/// @brief Safely converts a Cell to Player (returns nullopt for Empty)
constexpr std::optional<Player> cellToPlayer(Cell c) noexcept
{
    if (c == Cell::Empty)
        return std::nullopt;
    return c == Cell::Black ? Player::Black : Player::White;
}

constexpr Player opponent(Player p) noexcept
{
    return p == Player::Black ? Player::White : Player::Black;
}

/// @brief Position on the game board (0-based coordinates)
struct Pos {
    uint8_t x { 0 }, y { 0 }; // 0..18

    /// @brief Equality comparison
    constexpr bool operator==(const Pos& other) const noexcept { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Pos& other) const noexcept { return !(*this == other); }

    /// @brief Check if position is within board bounds
    constexpr bool isValid() const noexcept { return x < BOARD_SIZE && y < BOARD_SIZE; }

    /// @brief Convert to linear index (for arrays)
    constexpr uint16_t toIndex() const noexcept { return y * BOARD_SIZE + x; }

    /// @brief Create position from linear index
    static constexpr Pos fromIndex(uint16_t idx) noexcept
    {
        return { static_cast<uint8_t>(idx % BOARD_SIZE), static_cast<uint8_t>(idx / BOARD_SIZE) };
    }
};

/// @brief Represents a move in the game
struct Move {
    Pos pos {};
    Player by { Player::Black };

    /// @brief Check if the move is valid (position within bounds)
    constexpr bool isValid() const noexcept { return pos.isValid(); }

    /// @brief Equality comparison
    constexpr bool operator==(const Move& other) const noexcept
    {
        return pos == other.pos && by == other.by;
    }
};

/// @brief Represents the rules of the game
struct RuleSet {
    bool forbidDoubleThree = true;
    bool allowFiveOrMore = true;
    bool capturesEnabled = true;
    uint8_t captureWinPairs = 5; // 5 pairs = 10 stones
};

/// @brief Represents the configuration for the engine
struct EngineConfig {
    RuleSet rules {}; // Game rules
    int maxDepthHint = 6; // Maximum depth for search
    int timeBudgetMs = 450; // Time budget in milliseconds
    std::size_t ttBytes = 64ull << 20; // 64 MB
    unsigned long long nodeCap = 0; // 0 = infinite (strict control by time)
    uint32_t randomSeed = 0; // Seed for the random number generator
};

/// @brief Represents captured stone pairs count
struct CaptureCount {
    int black = 0;
    int white = 0;

    constexpr bool operator==(const CaptureCount& other) const noexcept
    {
        return black == other.black && white == other.white;
    }
};

/// @brief Result of attempting to play a move
struct PlayResult {
    bool success = false;
    std::string error; // Empty if successful

    constexpr explicit operator bool() const noexcept { return success; }

    static PlayResult ok() { return { true, "" }; }
    static PlayResult fail(std::string reason) { return { false, std::move(reason) }; }
};

/// @brief Game status/outcome
enum class GameStatus {
    Ongoing, // Game is still in progress
    WinByAlign, // Won by aligning 5 stones
    WinByCapture, // Won by capturing 5 pairs (10 stones)
    Draw // Game ended in a draw
};

/// @brief Complete game state for serialization/persistence
struct GameState {
    std::array<Cell, BOARD_SIZE * BOARD_SIZE> board;
    std::vector<Move> moveHistory;
    Player currentPlayer = Player::Black;
    CaptureCount captures;
    GameStatus status = GameStatus::Ongoing;
    RuleSet rules;

    // Helper to create from current game
    static GameState fromBoard(const class IBoardView& boardView, const std::vector<Move>& history, const RuleSet& gameRules);
};

/// @brief Type alias for Position (more semantic for some contexts)
using Position = Pos;

// I/O operators for debugging (implemented in src/application/Types.cpp)
std::ostream& operator<<(std::ostream& os, Player p);
std::ostream& operator<<(std::ostream& os, Cell c);
std::ostream& operator<<(std::ostream& os, const Pos& pos);
std::ostream& operator<<(std::ostream& os, const Move& move);

} // namespace gomoku
