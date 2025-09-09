#pragma once
#include "gomoku/BoardView.hpp"
#include "gomoku/Types.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

// Implémentation concrète de BoardView pour libgomoku_core.a
// API publique : voir include/gomoku/BoardView.hpp
// Représentation métier sans dépendance UI
class Board final : public BoardView {
public:
    Board();

    // ---- BoardView ----
    Cell at(uint8_t x, uint8_t y) const override;
    Player toPlay() const override { return side; }
    std::pair<int, int> capturedPairs() const override { return { blackPairs, whitePairs }; }
    GameStatus status() const override { return state; }

    // ---- API ----
    void reset();
    bool isInside(int x, int y) const { return (0 <= x && x < BOARD_SIZE && 0 <= y && y < BOARD_SIZE); }
    bool isEmpty(uint8_t x, uint8_t y) const { return isInside(x, y) && cells[idx(x, y)] == Cell::Empty; }

    bool play(Move m, const RuleSet& rules, std::string* whyNot);
    bool undo();

    std::vector<Move> legalMoves(Player p, const RuleSet& rules) const;

    // Zobrist
    uint64_t zobristKey() const { return zobrist_; }

    // Gestion explicite du trait
    void forceSide(Player p);

    // Utilitaire: plein ?
    bool isBoardFull() const;

private:
    static constexpr int N = BOARD_SIZE * BOARD_SIZE;
    static constexpr int idx(int x, int y) { return y * BOARD_SIZE + x; }

    std::array<Cell, N> cells {};
    Player side { Player::Black };
    int blackPairs { 0 }, whitePairs { 0 };
    GameStatus state { GameStatus::Ongoing };

    struct UndoEntry {
        Move move {};
        std::vector<Pos> removed; // pierres capturées
        int blackPairsBefore { 0 }, whitePairsBefore { 0 };
        GameStatus stateBefore { GameStatus::Ongoing };
        Player sideBefore { Player::Black };
    };
    std::vector<UndoEntry> history;

    // --- Zobrist ---
    uint64_t zobrist_ = 0;

    // --- Règles / détections ---
    bool createsIllegalDoubleThree(Move m, const RuleSet& rules) const;
    bool checkFiveOrMoreFrom(Pos p, Cell who) const;
    int applyCapturesAround(Pos p, Cell who, const RuleSet& rules, std::vector<Pos>& removed);

    bool hasAnyFive(Cell who) const;
    bool isFiveBreakableNow(Player justPlayed, const RuleSet& rules) const;

    bool wouldCapture(Move m) const;
};

} // namespace gomoku
