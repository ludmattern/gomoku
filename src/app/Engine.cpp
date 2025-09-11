#include "gomoku/Engine.hpp"
#include "ai/Search.hpp"
#include "core/Board.hpp"
#include <utility>

namespace gomoku {

struct Engine::Impl {
    EngineConfig cfg {};
    Board board {};
    Search search { { cfg.timeBudgetMs, cfg.maxDepthHint, cfg.ttBytes, cfg.nodeCap } };
};

Engine::Engine(const EngineConfig& cfg)
    : self(new Impl)
{
    self->cfg = cfg;
    self->search = Search { { cfg.timeBudgetMs, cfg.maxDepthHint, cfg.ttBytes, cfg.nodeCap } };
}

Engine::~Engine() = default;

const ABoardView& Engine::board() const { return self->board; }

void Engine::reset() { self->board.reset(); }

bool Engine::isLegal(Move m, std::string* whyNot) const
{
    if (m.by != self->board.toPlay()) {
        if (whyNot)
            *whyNot = "Not this player's turn.";
        return false;
    }
    // On réutilise play/undo en sandbox — simple mais sûr (on pourrait faire plus performant)
    Board copy = self->board;
    if (!copy.play(m, self->cfg.rules, whyNot))
        return false;
    return true;
}

bool Engine::play(Move m)
{
    std::string why;
    return self->board.play(m, self->cfg.rules, &why);
}

bool Engine::undo() { return self->board.undo(); }

void Engine::forceSide(Player p) { self->board.forceSide(p); }

std::optional<Move> Engine::bestMove(int /*timeBudgetMs*/, SearchStats* outStats) const
{
    SearchStats tmp {};
    Board& nb = const_cast<Board&>(self->board);
    auto res = self->search.bestMove(nb, self->cfg.rules, &tmp);
    if (outStats)
        *outStats = tmp;
    return res;
}

std::vector<Move> Engine::legalMoves() const
{
    return self->board.legalMoves(self->board.toPlay(), self->cfg.rules);
}

} // namespace gomoku
