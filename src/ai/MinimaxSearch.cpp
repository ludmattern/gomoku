#include "gomoku/ai/MinimaxSearch.hpp"
#include "gomoku/core/Board.hpp"
#include <algorithm>
#include <limits>

namespace gomoku {

// Note: cellOf and other are now available as playerToCell and opponent in Types.hpp

std::optional<Move> MinimaxSearch::bestMove(Board& board, const RuleSet& rules, SearchStats* stats)
{
    if (stats)
        *stats = SearchStats {};

    // Ouverture: centre si plateau vide
    bool empty = true;
    for (uint8_t y = 0; y < BOARD_SIZE && empty; ++y)
        for (uint8_t x = 0; x < BOARD_SIZE; ++x)
            if (board.at(x, y) != Cell::Empty) {
                empty = false;
                break;
            }
    if (empty) {
        Move c { { (uint8_t)(BOARD_SIZE / 2), (uint8_t)(BOARD_SIZE / 2) }, board.toPlay() };
        if (stats) {
            stats->nodes = 1;
            stats->depthReached = 0;
            stats->timeMs = 0;
            stats->principalVariation = { c };
        }
        return c;
    }

    budgetMs = cfg.timeBudgetMs;
    t0 = std::chrono::steady_clock::now();
    timeUp = false;
    visited = 0;
    initTT(cfg.ttBytes);
    nodeCap = cfg.nodeCap;

    std::optional<Move> best {};
    int bestScore = std::numeric_limits<int>::min();

    // Fallback rapide: premier légal (cas extrême)
    std::vector<Move> legals = board.legalMoves(board.toPlay(), rules);
    if (legals.empty()) {
        if (stats) {
            stats->nodes = 0;
            stats->depthReached = 0;
            stats->principalVariation.clear();
        }
        return std::nullopt;
    } else {
        best = legals.front();
        bestScore = -1'000'000;
        if (stats) {
            stats->principalVariation = { *best };
        }
    }

    // Iterative deepening
    int maxDepth = std::max(2, cfg.maxDepthHint);
    for (int depth = 2; depth <= maxDepth; ++depth) {
        if (expired()) {
            timeUp = true;
            break;
        }
        auto res = alphabeta(board, rules, depth,
            std::numeric_limits<int>::min() / 2,
            std::numeric_limits<int>::max() / 2,
            board.toPlay(), stats);
        if (timeUp)
            break;
        if (res.move) {
            best = res.move;
            bestScore = res.score;
            if (stats) {
                stats->depthReached = depth;
                stats->principalVariation = { *res.move };
            }
            if (bestScore > 800000)
                break;
        }
    }

    if (stats) {
        using namespace std::chrono;
        stats->timeMs = (int)duration_cast<milliseconds>(steady_clock::now() - t0).count();
    }
    return best;
}

// ---------------- TT ----------------
void MinimaxSearch::initTT(std::size_t bytes)
{
    if (!bytes)
        bytes = (16ull << 20);
    std::size_t n = bytes / sizeof(TTEntry);
    if (n < 1024)
        n = 1024;
    std::size_t pow2 = 1;
    while (pow2 < n)
        pow2 <<= 1;
    tt.assign(pow2, TTEntry {});
    ttMask = pow2 - 1;
}

MinimaxSearch::TTEntry* MinimaxSearch::ttProbe(uint64_t key) const
{
    if (tt.empty())
        return nullptr;
    return const_cast<TTEntry*>(&tt[key & ttMask]);
}

void MinimaxSearch::ttStore(uint64_t key, int depth, int score, TTFlag flag, const std::optional<Move>& best)
{
    if (tt.empty())
        return;
    auto& e = tt[key & ttMask];
    if (e.key != key || depth >= e.depth) {
        e.key = key;
        e.depth = depth;
        e.score = score;
        e.flag = flag;
        e.best = best.value_or(Move { Pos { 0, 0 }, Player::Black });
    }
}

// ---------------- Alpha-Beta ----------------
MinimaxSearch::ABResult MinimaxSearch::alphabeta(Board& b, const RuleSet& rules, int depth, int alpha, int beta, Player maxPlayer, SearchStats* stats)
{
    if (stats)
        ++stats->nodes;
    ++visited;
    if (expired()) {
        timeUp = true;
        return { evaluate(b, maxPlayer), std::nullopt };
    }

    uint64_t key = b.zobristKey();
    int alpha0 = alpha;
    Move ttBest {};
    bool haveTTBest = false;

    if (auto e = ttProbe(key)) {
        if (e->key == key && e->depth >= depth) {
            if (stats)
                ++stats->ttHits;
            if (e->flag == TTFlag::Exact)
                return { e->score, e->best };
            if (e->flag == TTFlag::Lower)
                alpha = std::max(alpha, e->score);
            else
                beta = std::min(beta, e->score);
            if (alpha >= beta)
                return { e->score, e->best };
        }
        if (e->key == key) {
            ttBest = e->best;
            haveTTBest = true;
        }
    }

    // Fin de partie ?
    if (b.status() != GameStatus::Ongoing) {
        int val = 0;
        if (b.status() == GameStatus::WinByAlign || b.status() == GameStatus::WinByCapture) {
            Player winner = opponent(b.toPlay());
            val = (winner == maxPlayer) ? 1'000'000 : -1'000'000;
        }
        return { val, std::nullopt };
    }

    if (depth == 0) {
        int val = evaluate(b, maxPlayer);
        return { val, std::nullopt };
    }

    Player toPlay = b.toPlay();
    auto moves = orderedMoves(b, rules, toPlay);
    if (haveTTBest) {
        for (size_t i = 0; i < moves.size(); ++i) {
            if (moves[i].pos.x == ttBest.pos.x && moves[i].pos.y == ttBest.pos.y) {
                std::swap(moves[0], moves[i]);
                break;
            }
        }
    }
    if (moves.empty())
        return { 0, std::nullopt };

    std::optional<Move> bestMove {};

    if (toPlay == maxPlayer) {
        int best = std::numeric_limits<int>::min();
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            b.undo();
            if (timeUp)
                break;
            if (child.score > best) {
                best = child.score;
                bestMove = m;
            }
            alpha = std::max(alpha, child.score);
            if (alpha >= beta)
                break;
        }
        // TT store
        TTFlag flag = TTFlag::Exact;
        if (best <= alpha0)
            flag = TTFlag::Upper;
        else if (best >= beta)
            flag = TTFlag::Lower;
        ttStore(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    } else {
        int best = std::numeric_limits<int>::max();
        for (const auto& m : moves) {
            if (expired()) {
                timeUp = true;
                break;
            }
            if (!b.play(m, rules, nullptr))
                continue;
            auto child = alphabeta(b, rules, depth - 1, alpha, beta, maxPlayer, stats);
            b.undo();
            if (timeUp)
                break;
            if (child.score < best) {
                best = child.score;
                bestMove = m;
            }
            beta = std::min(beta, child.score);
            if (beta <= alpha)
                break;
        }
        TTFlag flag = TTFlag::Exact;
        if (best <= alpha0)
            flag = TTFlag::Upper;
        else if (best >= beta)
            flag = TTFlag::Lower;
        ttStore(key, depth, best, flag, bestMove);
        return { bestMove ? best : evaluate(b, maxPlayer), bestMove };
    }
}

// ---------------- Move ordering (léger) ----------------
std::vector<Move> MinimaxSearch::orderedMoves(Board& b, const RuleSet& rules, Player toPlay) const
{
    (void)rules;
    auto ms = b.legalMoves(toPlay, rules);
    if (ms.size() <= 1)
        return ms;

    struct Sc {
        Move m;
        int s;
    };
    std::vector<Sc> scored;
    scored.reserve(ms.size());

    for (size_t i = 0; i < ms.size(); ++i) {
        if (expired()) {
            for (size_t j = i; j < ms.size(); ++j)
                scored.push_back({ ms[j], -100 });
            break;
        }
        const auto& m = ms[i];
        int s = quickScoreMove(b, toPlay, m.pos.x, m.pos.y);
        scored.push_back({ m, s });
    }

    std::stable_sort(scored.begin(), scored.end(),
        [](const Sc& a, const Sc& b) { return a.s > b.s; });

    std::vector<Move> out;
    out.reserve(scored.size());
    for (auto& e : scored)
        out.push_back(e.m);
    return out;
}

// Score local rapide sans play()
int MinimaxSearch::quickScoreMove(const Board& b, Player toPlay, uint8_t x, uint8_t y) const
{
    const Cell me = (toPlay == Player::Black ? Cell::Black : Cell::White);
    const Cell opp = (me == Cell::Black ? Cell::White : Cell::Black);
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    if (b.at(x, y) != Cell::Empty)
        return -1'000'000;

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    int score = 0;
    score -= 3 * manhattan(x, y, BOARD_SIZE / 2, BOARD_SIZE / 2);

    for (int d = 0; d < 4; ++d) {
        int len = 1;
        bool openA = false, openB = false;

        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && b.at(xx, yy) == me) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && b.at(xx, yy) == Cell::Empty;

        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && b.at(xx, yy) == me) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && b.at(xx, yy) == Cell::Empty;

        int open = (openA ? 1 : 0) + (openB ? 1 : 0);

        if (len >= 5)
            score += 900000;
        else if (len == 4)
            score += (open == 2 ? 120000 : 30000);
        else if (len == 3)
            score += (open == 2 ? 12000 : 3000);
        else if (len == 2)
            score += (open == 2 ? 1000 : 300);
        else
            score += 20;
    }

    // opportunité de capture XOOX
    for (int d = 0; d < 4; ++d) {
        int x1 = (int)x + DX[d], y1 = (int)y + DY[d];
        int x2 = (int)x + 2 * DX[d], y2 = (int)y + 2 * DY[d];
        int x3 = (int)x + 3 * DX[d], y3 = (int)y + 3 * DY[d];
        if (inside(x3, y3) && b.at(x1, y1) == opp && b.at(x2, y2) == opp && b.at(x3, y3) == me)
            score += 4000;

        int X1 = (int)x - DX[d], Y1 = (int)y - DY[d];
        int X2 = (int)x - 2 * DX[d], Y2 = (int)y - 2 * DY[d];
        int X3 = (int)x - 3 * DX[d], Y3 = (int)y - 3 * DY[d];
        if (inside(X3, Y3) && b.at(X1, Y1) == opp && b.at(X2, Y2) == opp && b.at(X3, Y3) == me)
            score += 4000;
    }

    return score;
}

// ---------------- Evaluation ----------------
int MinimaxSearch::evaluate(const Board& b, Player pov) const
{
    auto scoreSide = [&](Player p) -> int {
        int sum = 0;
        Cell who = playerToCell(p);
        for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
            for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
                if (b.at(x, y) != who)
                    continue;
                sum += evaluateOneDir(b, x, y, who, 1, 0);
                sum += evaluateOneDir(b, x, y, who, 0, 1);
                sum += evaluateOneDir(b, x, y, who, 1, 1);
                sum += evaluateOneDir(b, x, y, who, 1, -1);
            }
        }
        auto [bp, wp] = b.capturedPairs();
        int pairs = (p == Player::Black) ? (bp - wp) : (wp - bp);
        sum += pairs * 3000;
        return sum;
    };

    int me = scoreSide(pov);
    int opp = scoreSide(opponent(pov));
    return me - opp;
}

int MinimaxSearch::evaluateOneDir(const Board& b, uint8_t x, uint8_t y, Cell who, int dx, int dy) const
{
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    int X = x, Y = y, len = 1;
    int x1 = X - dx, y1 = Y - dy;
    while (inside(x1, y1) && b.at(x1, y1) == who) {
        ++len;
        x1 -= dx;
        y1 -= dy;
    }
    bool openA = inside(x1, y1) && b.at(x1, y1) == Cell::Empty;

    int x2 = X + dx, y2 = Y + dy;
    while (inside(x2, y2) && b.at(x2, y2) == who) {
        ++len;
        x2 += dx;
        y2 += dy;
    }
    bool openB = inside(x2, y2) && b.at(x2, y2) == Cell::Empty;

    int open = (openA ? 1 : 0) + (openB ? 1 : 0);

    if (len >= 5)
        return 500000;
    if (len == 4) {
        if (open == 2)
            return 120000;
        if (open == 1)
            return 30000;
    }
    if (len == 3) {
        if (open == 2)
            return 12000;
        if (open == 1)
            return 3000;
    }
    if (len == 2) {
        if (open == 2)
            return 1000;
        if (open == 1)
            return 300;
    }
    return 20;
}

} // namespace gomoku
