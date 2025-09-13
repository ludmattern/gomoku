#include "gomoku/application/MoveValidator.hpp"
#include "gomoku/core/Types.hpp"

namespace gomoku::application {

MoveValidator::Result MoveValidator::validate(const IBoardView& board, const RuleSet& /*rules*/, const Move& move) const
{
    Result r;

    if (!move.isValid()) {
        r.reason = "Invalid position";
        return r;
    }
    if (board.status() != GameStatus::Ongoing) {
        r.reason = "Game already finished";
        return r;
    }
    if (move.by != board.toPlay()) {
        r.reason = "Not this player's turn";
        return r;
    }
    if (board.at(move.pos.x, move.pos.y) != Cell::Empty) {
        r.reason = "Position already occupied";
        return r;
    }

    // Prévalidation réussie; les règles complexes (double-trois, etc.) seront
    // vérifiées par l'application réelle du coup dans Board::tryPlay.
    r.ok = true;
    return r;
}

} // namespace gomoku::application
