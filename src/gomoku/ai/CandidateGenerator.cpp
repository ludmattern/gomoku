// gomoku/ai/CandidateGenerator.cpp
#include "gomoku/ai/CandidateGenerator.hpp"
#include "gomoku/core/Logger.hpp"
#include <algorithm>
#include <array>
#include <bitset>

namespace gomoku {

namespace {

    //-------------------------------------------
    // Types & utilitaires bas niveau
    //-------------------------------------------
    struct Rect {
        int x1, y1, x2, y2;
    };
    struct P {
        uint8_t x, y;
    };

    constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

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

    //-------------------------------------------
    // Étape 0 — Collecte des pierres
    //-------------------------------------------
    void collectStones(const Board& b, std::vector<P>& out)
    {
        out.clear();
        const auto& occ = b.occupiedPositions();
        out.reserve(occ.size());
        for (const auto& pos : occ)
            out.push_back(P { pos.x, pos.y });
    }

    //-------------------------------------------
    // Étape 1 — Îlots par proximité Chebyshev
    //-------------------------------------------
    std::vector<Rect> buildIslands(const std::vector<P>& stones, uint8_t gap)
    {
        const int n = (int)stones.size();
        std::vector<uint8_t> vis(n, 0);
        std::vector<Rect> rects;
        rects.reserve(16);

        auto nearCheb = [&](const P& a, const P& b) -> bool {
            int dx = std::abs((int)a.x - (int)b.x);
            int dy = std::abs((int)a.y - (int)b.y);
            return std::max(dx, dy) <= gap;
        };

        int islandCount = 0;
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
                islandCount++;
            }
        return rects;
    }

    //-------------------------------------------
    // Étape 2 — Fusion rectangulaire
    //-------------------------------------------
    std::vector<Rect> mergeAll(std::vector<Rect> rs)
    {
        int mergeCount = 0;
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t i = 0; i < rs.size(); ++i) {
                for (size_t j = i + 1; j < rs.size(); ++j) {
                    if (intersect(rs[i], rs[j])) {
                        rs[i] = merge(rs[i], rs[j]);
                        rs.erase(rs.begin() + j);
                        changed = true;
                        mergeCount++;
                        goto nextOuter;
                    }
                }
            }
        nextOuter:;
        }
        return rs;
    }

    std::vector<Rect> dilateAndMerge(std::vector<Rect> rs, int effMargin)
    {
        for (auto& r : rs)
            r = dilate(r, effMargin);
        return mergeAll(std::move(rs));
    }

    //-------------------------------------------
    // Étape 3 — Masque O(1) des zones actives
    //-------------------------------------------
    using ActiveMask = std::array<uint8_t, BOARD_CELLS>;
    ActiveMask buildActiveMask(const std::vector<Rect>& rects)
    {
        ActiveMask active {}; // zéro-initialisé
        for (const auto& r : rects)
            for (int y = r.y1; y <= r.y2; ++y)
                for (int x = r.x1; x <= r.x2; ++x)
                    active[y * BOARD_SIZE + x] = 1;
        return active;
    }

    //-------------------------------------------
    // Étape 4 — Offsets diamant (cache thread_local)
    //-------------------------------------------
    using Offset = std::pair<int8_t, int8_t>;

    const std::vector<Offset>& diamondOffsets(int ringR)
    {
        static thread_local int cachedRingR = -1;
        static thread_local std::vector<Offset> DIAMOND;
        if (cachedRingR != ringR) {
            cachedRingR = ringR;
            DIAMOND.clear();
            DIAMOND.reserve((2 * ringR + 1) * (2 * ringR + 1));
            for (int dy = -ringR; dy <= ringR; ++dy) {
                int rem = ringR - std::abs(dy);
                for (int dx = -rem; dx <= rem; ++dx)
                    DIAMOND.emplace_back((int8_t)dx, (int8_t)dy);
            }
        }
        return DIAMOND;
    }

    //-------------------------------------------
    // Déduplication (bitset compact)
    //-------------------------------------------
    using SeenSet = std::bitset<BOARD_CELLS>;
    inline bool markIfNew(SeenSet& seen, int x, int y)
    {
        const int i = y * BOARD_SIZE + x;
        if (seen.test(i))
            return false;
        seen.set(i);
        return true;
    }

    //-------------------------------------------
    // Étape 5 — Émission des candidats par anneaux
    //-------------------------------------------
    void emitNeighborhood(const Board& b,
        const ActiveMask& active,
        const std::vector<Offset>& ring,
        uint8_t cx, uint8_t cy,
        Player toPlay,
        uint16_t maxCandidates,
        SeenSet& seen,
        std::vector<Move>& out)
    {
        for (auto [dx, dy] : ring) {
            int x = (int)cx + dx, y = (int)cy + dy;
            if ((unsigned)x >= BOARD_SIZE || (unsigned)y >= BOARD_SIZE)
                continue;
            int idx = y * BOARD_SIZE + x;
            if (!active[idx])
                continue; // clamp O(1)
            if (b.at((uint8_t)x, (uint8_t)y) != Cell::Empty)
                continue;
            if (!markIfNew(seen, x, y))
                continue;
            out.push_back(Move { { (uint8_t)x, (uint8_t)y }, toPlay });
            if (out.size() >= maxCandidates)
                return;
        }
    }

    void generateFromRings(const Board& b,
        const std::vector<P>& stones,
        const ActiveMask& active,
        Player toPlay,
        const CandidateConfig& cfg,
        SeenSet& seen,
        std::vector<Move>& out)
    {
        const auto& ring = diamondOffsets(cfg.ringR);
        out.reserve(cfg.maxCandidates);

        if (cfg.includeOpponentRing) {
            for (const auto& p : stones) {
                emitNeighborhood(b, active, ring, p.x, p.y, toPlay, cfg.maxCandidates, seen, out);
                if (out.size() >= cfg.maxCandidates)
                    return;
            }
        } else {
            const Cell mine = (toPlay == Player::Black ? Cell::Black : Cell::White);
            for (const auto& p : stones) {
                if (b.at(p.x, p.y) != mine)
                    continue;
                emitNeighborhood(b, active, ring, p.x, p.y, toPlay, cfg.maxCandidates, seen, out);
                if (out.size() >= cfg.maxCandidates)
                    return;
            }
        }
    }

    //-------------------------------------------
    // Étape 6 — Fallback scan des rectangles
    //-------------------------------------------
    void fallbackScan(const Board& b,
        const std::vector<Rect>& rects,
        const ActiveMask& active,
        Player toPlay,
        const CandidateConfig& cfg,
        SeenSet& seen,
        std::vector<Move>& out)
    {
        for (const auto& r : rects) {
            for (int y = r.y1; y <= r.y2; ++y)
                for (int x = r.x1; x <= r.x2; ++x) {
                    if (b.at((uint8_t)x, (uint8_t)y) != Cell::Empty)
                        continue;
                    if (!active[y * BOARD_SIZE + x])
                        continue; // cohérence
                    if (!markIfNew(seen, x, y))
                        continue;
                    out.push_back(Move { { (uint8_t)x, (uint8_t)y }, toPlay });
                    if (out.size() >= cfg.maxCandidates)
                        return;
                }
        }
    }

    //-------------------------------------------
    // Étape 7 — Finalisation (cap + alertes)
    //-------------------------------------------
    void finalizeCandidates(std::vector<Move>& out, uint16_t maxCandidates)
    {
        if (out.size() > maxCandidates) {
            out.resize(maxCandidates);
        }
    }

    //-------------------------------------------
    // Utilitaire — Plateau vide ?
    //-------------------------------------------
    bool isEmptyBoard(const Board& b)
    {
        return b.occupiedPositions().empty();
    }

} // namespace

//-------------------------------------------
// API — Pipeline lisible
//-------------------------------------------
std::vector<Move> CandidateGenerator::generate(const Board& b, const RuleSet& rules,
    Player toPlay, const CandidateConfig& cfg)
{
    (void)rules; // légalité fine = moteur play/undo

    // 0) Plateau vide -> centre
    if (isEmptyBoard(b)) {
        LOG_INFO("Empty board detected - center move");
        Move c { { (uint8_t)(BOARD_SIZE / 2), (uint8_t)(BOARD_SIZE / 2) }, toPlay };
        return { c };
    }

    // 1) Collecte des pierres
    std::vector<P> stones;
    collectStones(b, stones);

    // 2) Îlots (BFS Chebyshev) -> dilatation(>=ringR) -> fusion
    auto rects = buildIslands(stones, cfg.groupGap);
    const int effMargin = std::max<int>(cfg.margin, cfg.ringR);
    rects = dilateAndMerge(std::move(rects), effMargin);

    // 3) Masque des zones actives
    ActiveMask active = buildActiveMask(rects);

    // 4–5) Anneaux (Manhattan <= ringR) clampés par masque + dédup bitset
    SeenSet seen;
    std::vector<Move> out;
    generateFromRings(b, stones, active, toPlay, cfg, seen, out);

    // 6) Fallback scan si densité insuffisante
    if (out.size() < 12) {
        fallbackScan(b, rects, active, toPlay, cfg, seen, out);
    }

    // 7) Finalisation
    finalizeCandidates(out, cfg.maxCandidates);
    return out;
}

} // namespace gomoku
