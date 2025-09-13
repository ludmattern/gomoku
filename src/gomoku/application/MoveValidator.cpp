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

    // Prévalidation réussie; les règles complexes + tour + occupation sont
    // vérifiées définitivement lors de l'appel à Board::tryPlay.
    r.ok = true;
    return r;
}

} // namespace gomoku::application
