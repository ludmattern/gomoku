// gomoku/ai/QuiescenceSearch.cpp
#include "gomoku/ai/QuiescenceSearch.hpp"
#include "gomoku/ai/CandidateGenerator.hpp"
#include "gomoku/core/Board.hpp"
#include <algorithm>
#include <functional>

namespace gomoku {

// --- helper d'ordonnancement très léger (inline) ---
static inline int qOrderKey(const Board& b, const Move& m, Player toPlay)
{
    // Heuristique cheap: priorité si case adjacente immédiate à une pierre,
    // bonus si la pose gagne/bloque en 1 (détecté via motifs rapides).
    // NB: volontairement simple (O(1) voisinage).
    int key = 0;
    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (dx || dy) {
                int X = (int)m.pos.x + dx, Y = (int)m.pos.y + dy;
                if (inside(X, Y) && b.at((uint8_t)X, (uint8_t)Y) != Cell::Empty)
                    key += 1;
            }
    // Légers bonus côté trait
    (void)toPlay;
    return key;
}

QuiescenceSearch::QuiescenceSearch()
    : config_ {}
{
}
QuiescenceSearch::QuiescenceSearch(const Config& config)
    : config_(config)
{
}

QuiescenceSearch::Result QuiescenceSearch::search(
    Board& board, const RuleSet& rules, int alpha, int beta, Player maxPlayer,
    std::function<int(const Board&, Player)> evaluateFunction,
    SearchStats* stats, int depth)
{
    if (stats)
        ++stats->qnodes;
    ++visitedNodes_;
    if (shouldStop(stats)) {
        return { evaluateFunction(board, maxPlayer), std::nullopt };
    }

    // --- borne profondeur quiescence ---
    if (depth >= config_.maxDepth) {
        return { evaluateFunction(board, maxPlayer), std::nullopt };
    }

    // --- terminal avant toute évaluation lourde ---
    GameStatus st = board.status();
    if (st != GameStatus::Ongoing) {
        int val = 0;
        if (st == GameStatus::WinByAlign || st == GameStatus::WinByCapture) {
            Player winner = opponent(board.toPlay());
            val = (winner == maxPlayer) ? 1'000'000 : -1'000'000;
        }
        return { val, std::nullopt };
    }

    // --- stand pat ---
    const int standPat = evaluateFunction(board, maxPlayer);

    Player toPlay = board.toPlay();

    // --- fail-soft coups standards sur standPat ---
    if (toPlay == maxPlayer) {
        if (standPat >= beta)
            return { standPat, std::nullopt };
        if (standPat > alpha)
            alpha = standPat;
    } else {
        if (standPat <= alpha)
            return { standPat, std::nullopt };
        if (standPat < beta)
            beta = standPat;
    }

    // --- génération unique des coups tactiques ---
    auto moves = generateTacticalMoves(board, rules, toPlay);

    // position calme => standPat (supprime le double scan d'avant)
    if (moves.empty()) {
        return { standPat, std::nullopt };
    }

    // --- delta pruning très simple ---
    // Suppose qu’un seul coup tactique peut améliorer d’environ un open-4 (~120k).
    // Ajuste selon ton barème.
    const int delta = 120000;
    if (toPlay == maxPlayer && standPat + delta <= alpha) {
        return { standPat, std::nullopt };
    }
    if (toPlay != maxPlayer && standPat - delta >= beta) {
        return { standPat, std::nullopt };
    }

    // --- ordonnancement cheap pour booster les cutoffs ---
    std::sort(moves.begin(), moves.end(),
        [&](const Move& a, const Move& b) {
            return qOrderKey(board, a, toPlay) > qOrderKey(board, b, toPlay);
        });

    // --- NegaMax léger (réduit les branches) ---
    int best = standPat;
    std::optional<Move> bestMove;

    for (const auto& mv : moves) {
        if (shouldStop(stats))
            break;
        if (!board.play(mv, rules, nullptr))
            continue;

        // NegaMax: inverse les bornes et le signe en alternant max/min implicite
        auto res = search(board, rules, -beta, -alpha, maxPlayer,
            evaluateFunction, stats, depth + 1);
        board.undo();
        if (shouldStop(stats))
            break;

        int score = -res.score;

        if (score > best) {
            best = score;
            bestMove = mv;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break; // cutoff
        }
    }

    return { best, bestMove };
}

bool QuiescenceSearch::isQuiet(const Board& board, const RuleSet& rules, Player toPlay)
{
    if (!config_.enabled)
        return true;
    // NOTE: ne génère plus deux fois. Conservé pour compat API, mais non utilisé par search().
    auto tacticals = generateTacticalMoves(board, rules, toPlay);
    return tacticals.empty();
}

std::vector<Move> QuiescenceSearch::generateTacticalMoves(
    const Board& board, const RuleSet& rules, Player toPlay)
{
    CandidateConfig config;
    // Conseil: ici tu peux basculer vers un énumérateur « tactique only »
    // pour éviter un full CandidateGenerator si besoin.
    auto allMoves = CandidateGenerator::generate(board, rules, toPlay, config);

    std::vector<Move> tactical;
    tactical.reserve(config_.maxTacticalMoves);

    // Filtre tactique (quick patterns) + early-out
    for (const auto& m : allMoves) {
        if (isTacticalMove(board, m.pos.x, m.pos.y, toPlay)) {
            tactical.push_back(m);
            if (tactical.size() >= (size_t)config_.maxTacticalMoves)
                break;
        }
    }
    return tactical;
}


bool QuiescenceSearch::isTacticalMove(const Board& board, uint8_t x, uint8_t y, Player toPlay) const
{
    if (board.at(x, y) != Cell::Empty)
        return false;

    const Cell me = (toPlay == Player::Black ? Cell::Black : Cell::White);
    const Cell opponent = (me == Cell::Black ? Cell::White : Cell::Black);

    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    for (int d = 0; d < 4; ++d) {
        // Vérifier les menaces pour le joueur actuel
        int len = 1;
        bool openA = false, openB = false;

        // Extension vers l'arrière
        int xx = (int)x - DX[d], yy = (int)y - DY[d];
        while (inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        // Extension vers l'avant
        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == me) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        int open = (openA ? 1 : 0) + (openB ? 1 : 0);

        // Menace de victoire directe (4+ en ligne)
        if (len >= 4) {
            return true;
        }

        // Menace forte (3 ouvert ou 4 semi-ouvert)
        if ((len == 3 && open == 2) || (len == 4 && open >= 1)) {
            return true;
        }

        // Vérifier les blocages de menaces adverses
        len = 1;
        openA = openB = false;

        // Extension vers l'arrière pour l'adversaire
        xx = (int)x - DX[d];
        yy = (int)y - DY[d];
        while (inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == opponent) {
            ++len;
            xx -= DX[d];
            yy -= DY[d];
        }
        openA = inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        // Extension vers l'avant pour l'adversaire
        xx = (int)x + DX[d];
        yy = (int)y + DY[d];
        while (inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == opponent) {
            ++len;
            xx += DX[d];
            yy += DY[d];
        }
        openB = inside(xx, yy) && board.at(static_cast<uint8_t>(xx), static_cast<uint8_t>(yy)) == Cell::Empty;

        open = (openA ? 1 : 0) + (openB ? 1 : 0);

        // Blocage de menace de victoire adverse
        if (len >= 4) {
            return true;
        }

        // Blocage de menace forte adverse
        if ((len == 3 && open == 2) || (len == 4 && open >= 1)) {
            return true;
        }

        // Vérifier les opportunités de capture (pattern XOOY)
        int x1 = (int)x + DX[d], y1 = (int)y + DY[d];
        int x2 = (int)x + 2 * DX[d], y2 = (int)y + 2 * DY[d];
        int x3 = (int)x + 3 * DX[d], y3 = (int)y + 3 * DY[d];
        if (inside(x3, y3) && board.at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == opponent && board.at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == opponent && board.at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == me) {
            return true;
        }

        int X1 = (int)x - DX[d], Y1 = (int)y - DY[d];
        int X2 = (int)x - 2 * DX[d], Y2 = (int)y - 2 * DY[d];
        int X3 = (int)x - 3 * DX[d], Y3 = (int)y - 3 * DY[d];
        if (inside(X3, Y3) && board.at(static_cast<uint8_t>(X1), static_cast<uint8_t>(Y1)) == opponent && board.at(static_cast<uint8_t>(X2), static_cast<uint8_t>(Y2)) == opponent && board.at(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) == me) {
            return true;
        }
    }

    return false;
}

bool QuiescenceSearch::shouldStop(SearchStats* stats) const
{
    (void)stats; // stats disponibles si besoin (ex: nœuds max)
    if (stopCallback_) {
        return stopCallback_();
    }
    return false;
}

} // namespace gomoku