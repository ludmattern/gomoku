#include "gomoku/ai/QuiescenceSearch.hpp"
#include "gomoku/ai/CandidateGenerator.hpp"
#include "gomoku/core/Board.hpp"
#include <algorithm>
#include <functional>

namespace gomoku {

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

    // Limiter la profondeur de quiescence
    if (depth >= config_.maxDepth) {
        return { evaluateFunction(board, maxPlayer), std::nullopt };
    }

    // Évaluation statique de la position
    int standPat = evaluateFunction(board, maxPlayer);

    // Fin de partie ?
    if (board.status() != GameStatus::Ongoing) {
        int val = 0;
        if (board.status() == GameStatus::WinByAlign || board.status() == GameStatus::WinByCapture) {
            Player winner = opponent(board.toPlay());
            val = (winner == maxPlayer) ? 1'000'000 : -1'000'000;
        }
        return { val, std::nullopt };
    }

    Player toPlay = board.toPlay();

    // Si la position est calme, retourner l'évaluation statique
    if (isQuiet(board, rules, toPlay)) {
        return { standPat, std::nullopt };
    }

    // Beta cutoff
    if (toPlay == maxPlayer && standPat >= beta) {
        return { beta, std::nullopt };
    }

    // Alpha cutoff
    if (toPlay != maxPlayer && standPat <= alpha) {
        return { alpha, std::nullopt };
    }

    // Mise à jour de alpha/beta pour le joueur
    if (toPlay == maxPlayer) {
        alpha = std::max(alpha, standPat);
    } else {
        beta = std::min(beta, standPat);
    }

    // Générer seulement les mouvements tactiques
    auto moves = generateTacticalMoves(board, rules, toPlay);
    if (moves.empty()) {
        return { standPat, std::nullopt };
    }

    std::optional<Move> bestMove {};

    if (toPlay == maxPlayer) {
        int best = standPat;
        for (const auto& move : moves) {
            if (shouldStop(stats))
                break;

            if (!board.play(move, rules, nullptr))
                continue;

            auto result = search(board, rules, alpha, beta, maxPlayer, evaluateFunction, stats, depth + 1);
            board.undo();

            if (shouldStop(stats))
                break;

            if (result.score > best) {
                best = result.score;
                bestMove = move;
            }
            alpha = std::max(alpha, result.score);
            if (alpha >= beta)
                break;
        }
        return { best, bestMove };
    } else {
        int best = standPat;
        for (const auto& move : moves) {
            if (shouldStop(stats))
                break;

            if (!board.play(move, rules, nullptr))
                continue;

            auto result = search(board, rules, alpha, beta, maxPlayer, evaluateFunction, stats, depth + 1);
            board.undo();

            if (shouldStop(stats))
                break;

            if (result.score < best) {
                best = result.score;
                bestMove = move;
            }
            beta = std::min(beta, result.score);
            if (beta <= alpha)
                break;
        }
        return { best, bestMove };
    }
}

bool QuiescenceSearch::isQuiet(const Board& board, const RuleSet& rules, Player toPlay)
{
    if (!config_.enabled)
        return true;

    // Une position est calme s'il n'y a pas de mouvements tactiquement critiques
    auto tacticals = generateTacticalMoves(board, rules, toPlay);
    return tacticals.empty();
}

std::vector<Move> QuiescenceSearch::generateTacticalMoves(const Board& board, const RuleSet& rules, Player toPlay)
{
    // Générer tous les mouvements candidats
    CandidateConfig config;
    auto allMoves = CandidateGenerator::generate(board, rules, toPlay, config);

    std::vector<Move> tactical;
    tactical.reserve(config_.maxTacticalMoves);

    // Filtrer les mouvements tactiquement importants
    for (const auto& move : allMoves) {
        if (isTacticalMove(board, move.pos.x, move.pos.y, toPlay)) {
            tactical.push_back(move);
        }
        // Limiter le nombre pour éviter l'explosion combinatoire
        if (tactical.size() >= static_cast<size_t>(config_.maxTacticalMoves))
            break;
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