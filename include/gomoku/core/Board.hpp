#pragma once
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IBoardView.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

// Implémentation concrète de IBoardView pour libgomoku_logic.a
class Board final : public IBoardView {
public:
    Board();

    // ---- IBoardView interface ----
    Cell at(uint8_t x, uint8_t y) const override;
    Player toPlay() const override { return currentPlayer; }
    CaptureCount capturedPairs() const override { return { blackPairs, whitePairs }; }
    GameStatus status() const override { return gameState; }
    bool isBoardFull() const override;
    std::vector<Move> legalMoves(Player p, const RuleSet& rules) const override;
    uint64_t zobristKey() const override { return zobristHash; }

    // ---- Board-specific API ----
    void reset();
    bool isInside(uint8_t x, uint8_t y) const { return x < BOARD_SIZE && y < BOARD_SIZE; }
    bool isEmpty(uint8_t x, uint8_t y) const { return isInside(x, y) && cells[idx(x, y)] == Cell::Empty; }

    // Stone count (tracked incrementally)
    int stoneCount(Player p) const { return (p == Player::Black) ? blackStones : whiteStones; }

    PlayResult tryPlay(Move m, const RuleSet& rules);
    bool undo();

    bool speculativeTry(Move m, const RuleSet& rules, PlayResult* out);

    // Legacy API for compatibility
    bool play(Move m, const RuleSet& rules, std::string* whyNot = nullptr)
    {
        auto result = tryPlay(m, rules);
        if (whyNot)
            *whyNot = result.error;
        return result.success;
    }

    // Force player turn (for specific game setups)
    void forceSide(Player p);

private:
    static constexpr int N = BOARD_SIZE * BOARD_SIZE;
    static constexpr uint16_t idx(uint8_t x, uint8_t y) { return static_cast<uint16_t>(y * BOARD_SIZE + x); }

    std::array<Cell, N> cells {};
    Player currentPlayer { Player::Black };
    int blackPairs { 0 }, whitePairs { 0 };
    int blackStones { 0 }, whiteStones { 0 }; // tracked counts
    GameStatus gameState { GameStatus::Ongoing };

    struct UndoEntry {
        Move move {};
        std::vector<Pos> capturedStones; // pierres capturées
        int blackPairsBefore { 0 }, whitePairsBefore { 0 };
        int blackStonesBefore { 0 }, whiteStonesBefore { 0 };
        GameStatus stateBefore { GameStatus::Ongoing };
        Player playerBefore { Player::Black };
    };
    std::vector<UndoEntry> moveHistory;

    // --- Zobrist hash ---
    uint64_t zobristHash = 0;

    // --- Règles / détections ---
    bool createsIllegalDoubleThree(Move m, const RuleSet& rules) const;
    bool checkFiveOrMoreFrom(Pos p, Cell who) const;
    int applyCapturesAround(Pos p, Cell who, const RuleSet& rules, std::vector<Pos>& removed);

    bool hasAnyFive(Cell who) const;
    bool isFiveBreakableNow(Player justPlayed, const RuleSet& rules) const;

    bool wouldCapture(Move m) const;

    // Facteur interne : logique partagée d'application. Si record=true, pousse UndoEntry.
    PlayResult applyCore(Move m, const RuleSet& rules, bool record);
};

} // namespace gomoku
