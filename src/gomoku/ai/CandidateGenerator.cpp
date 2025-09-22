// gomoku/ai/CandidateGenerator.cpp
#include "gomoku/ai/CandidateGenerator.hpp"
#include <algorithm>

namespace gomoku {

namespace {
    struct Rect {
        int x1, y1, x2, y2;
    }; // inclusifs

    inline bool inside(int x, int y)
    {
        return 0 <= x && x < BOARD_SIZE && 0 <= y && y < BOARD_SIZE;
    }

    inline bool intersect(const Rect& a, const Rect& b)
    {
        return !(a.x2 < b.x1 || b.x2 < a.x1 || a.y2 < b.y1 || b.y2 < a.y1);
    }

    inline Rect merge(const Rect& a, const Rect& b)
    {
        return { std::min(a.x1, b.x1), std::min(a.y1, b.y1),
            std::max(a.x2, b.x2), std::max(a.y2, b.y2) };
    }

    inline Rect dilate(const Rect& r, int m)
    {
        return { std::max(0, r.x1 - m), std::max(0, r.y1 - m),
            std::min<int>(BOARD_SIZE - 1, r.x2 + m),
            std::min<int>(BOARD_SIZE - 1, r.y2 + m) };
    }

    struct P {
        uint8_t x, y;
    };

    void collectStones(const Board& b, std::vector<P>& out)
    {
        out.clear();
        out.reserve(256);
        for (uint8_t y = 0; y < BOARD_SIZE; ++y)
            for (uint8_t x = 0; x < BOARD_SIZE; ++x)
                if (b.at(x, y) != Cell::Empty)
                    out.push_back({ x, y });
    }

    std::vector<Rect> buildIslands(const std::vector<P>& stones, uint8_t gap)
    {
        // BFS sur proximité Chebyshev ≤ gap
        const int n = (int)stones.size();
        std::vector<uint8_t> vis(n, 0);
        std::vector<Rect> rects;
        rects.reserve(16);

        auto nearCheb = [&](const P& a, const P& b) -> bool {
            int dx = std::abs((int)a.x - (int)b.x);
            int dy = std::abs((int)a.y - (int)b.y);
            return std::max(dx, dy) <= gap;
        };

        for (int i = 0; i < n; ++i)
            if (!vis[i]) {
                vis[i] = 1;
                Rect r { stones[i].x, stones[i].y, stones[i].x, stones[i].y };
                std::vector<int> q;
                q.push_back(i);
                for (size_t k = 0; k < q.size(); ++k) {
                    int u = q[k];
                    for (int v = 0; v < n; ++v)
                        if (!vis[v] && nearCheb(stones[u], stones[v])) {
                            vis[v] = 1;
                            q.push_back(v);
                            r.x1 = std::min<int>(r.x1, stones[v].x);
                            r.y1 = std::min<int>(r.y1, stones[v].y);
                            r.x2 = std::max<int>(r.x2, stones[v].x);
                            r.y2 = std::max<int>(r.y2, stones[v].y);
                        }
                }
                rects.push_back(r);
            }
        return rects;
    }

    std::vector<Rect> mergeAll(std::vector<Rect> rs)
    {
        // fusion itérative O(m^2) — m petit dans la pratique
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t i = 0; i < rs.size(); ++i) {
                for (size_t j = i + 1; j < rs.size(); ++j) {
                    if (intersect(rs[i], rs[j])) {
                        rs[i] = merge(rs[i], rs[j]);
                        rs.erase(rs.begin() + j);
                        changed = true;
                        goto nextOuter;
                    }
                }
            }
        nextOuter:;
        }
        return rs;
    }

} // namespace

std::vector<Move> CandidateGenerator::generate(const Board& b, const RuleSet& rules,
    Player toPlay, const CandidateConfig& cfg)
{
    (void)rules; // si des règles interdisent certains coups, le moteur les refusera à play()

    // 1) cas trivial : plateau vide → centre
    bool empty = true;
    for (uint8_t y = 0; y < BOARD_SIZE && empty; ++y)
        for (uint8_t x = 0; x < BOARD_SIZE; ++x)
            if (b.at(x, y) != Cell::Empty) {
                empty = false;
                break;
            }
    if (empty) {
        Move c { { (uint8_t)(BOARD_SIZE / 2), (uint8_t)(BOARD_SIZE / 2) }, toPlay };
        return { c };
    }

    // 2) îlots
    std::vector<P> stones;
    collectStones(b, stones);
    auto rects = buildIslands(stones, cfg.groupGap);
    for (auto& r : rects)
        r = dilate(r, cfg.margin);
    rects = mergeAll(std::move(rects));

    // 3) bitmap de déduplication
    std::vector<uint8_t> seen(BOARD_SIZE * BOARD_SIZE, 0);
    auto mark = [&](int x, int y) -> bool {
        int i = y * BOARD_SIZE + x;
        if (seen[i])
            return false;
        seen[i] = 1;
        return true;
    };

    // 4) collecte des candidats par anneaux
    std::vector<Move> out;
    out.reserve(128);

    auto emitNeighborhood = [&](uint8_t cx, uint8_t cy) {
        for (int dy = -cfg.ringR; dy <= cfg.ringR; ++dy)
            for (int dx = -cfg.ringR; dx <= cfg.ringR; ++dx) {
                int x = (int)cx + dx, y = (int)cy + dy;
                if (!inside(x, y))
                    continue;
                if (std::abs(dx) + std::abs(dy) > cfg.ringR)
                    continue; // manhattan
                if (b.at((uint8_t)x, (uint8_t)y) != Cell::Empty)
                    continue;
                if (!mark(x, y))
                    continue;
                out.push_back(Move { { (uint8_t)x, (uint8_t)y }, toPlay });
            }
    };

    // 4a) anneaux autour des pierres (deux joueurs)
    for (const auto& p : stones)
        emitNeighborhood(p.x, p.y);

    // 5) clamp aux rectangles (supprime lointains accidentels)
    if (!rects.empty()) {
        out.erase(std::remove_if(out.begin(), out.end(), [&](const Move& m) {
            for (const auto& r : rects)
                if (r.x1 <= m.pos.x && m.pos.x <= r.x2 && r.y1 <= m.pos.y && m.pos.y <= r.y2)
                    return false;
            return true;
        }),
            out.end());
    }

    // 6) fallback si trop peu : on ratisse finement dans les rectangles (scan)
    if (out.size() < 12) {
        for (const auto& r : rects) {
            for (int y = r.y1; y <= r.y2; ++y)
                for (int x = r.x1; x <= r.x2; ++x) {
                    if (b.at((uint8_t)x, (uint8_t)y) != Cell::Empty)
                        continue;
                    if (!mark(x, y))
                        continue;
                    out.push_back(Move { { (uint8_t)x, (uint8_t)y }, toPlay });
                    if (out.size() >= cfg.maxCandidates)
                        goto done;
                }
        }
    }

done:
    if (out.size() > cfg.maxCandidates)
        out.resize(cfg.maxCandidates);
    return out;
}

} // namespace gomoku
