#include "gomoku/GameSession.hpp"

namespace gomoku {

GameSession::GameSession(const RuleSet& rules, Controller black, Controller white)
    : rules_(rules)
    , black_(black)
    , white_(white)
{
    // reset par défaut côté Engine
}

GameSnapshot GameSession::snapshot() const
{
    auto& b = eng_.board();
    return GameSnapshot {
        .view = &b,
        .lastMove = last_,
        .toPlay = b.toPlay(),
        .captures = b.capturedPairs(),
        .status = b.status()
    };
}

void GameSession::setController(Player side, Controller c)
{
    if (side == Player::Black)
        black_ = c;
    else
        white_ = c;
}
Controller GameSession::controller(Player side) const
{
    return (side == Player::Black) ? black_ : white_;
}

PlayResult GameSession::playHuman(Pos p)
{
    Move m { p, eng_.board().toPlay() };
    std::string why;
    if (!eng_.isLegal(m, &why))
        return { false, why, std::nullopt, std::nullopt };
    eng_.play(m);
    last_ = p;
    return { true, {}, m, std::nullopt };
}

PlayResult GameSession::playAI(int timeMs)
{
    SearchStats st {};
    auto bm = eng_.bestMove(timeMs, &st);
    if (!bm)
        return { false, "No AI move", std::nullopt, st };
    eng_.play(*bm);
    last_ = bm->pos;
    return { true, {}, bm, st };
}

bool GameSession::undo(int halfMoves)
{
    bool any = false;
    for (int i = 0; i < halfMoves; ++i) {
        if (!eng_.undo())
            break;
        any = true;
    }
    if (any)
        last_.reset();
    return any;
}

void GameSession::reset(Player start)
{
    eng_.reset();
    eng_.forceSide(start);
    last_.reset();
}

std::optional<Move> GameSession::hint(int timeMs, SearchStats* outStats) const
{
    SearchStats st {};
    auto m = eng_.suggest(timeMs, &st);
    if (outStats)
        *outStats = st;
    return m;
}

} // namespace gomoku
