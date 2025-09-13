#include "gomoku/core/Board.hpp"
#include <array>
#include <cassert>
#include <random>
#include <string>

namespace gomoku {

// Note: cellOf and other are now available as playerToCell and opponent in Types.hpp

// ------------------ Zobrist ------------------
namespace {
    std::array<uint64_t, 2 * BOARD_SIZE * BOARD_SIZE> Z_PCS {};
    uint64_t Z_SIDE = 0;

    inline int flat(int x, int y) { return y * BOARD_SIZE + x; }
    inline uint64_t z_of(Cell c, int x, int y)
    {
        if (c == Cell::Black)
            return Z_PCS[0 * BOARD_SIZE * BOARD_SIZE + flat(x, y)];
        if (c == Cell::White)
            return Z_PCS[1 * BOARD_SIZE * BOARD_SIZE + flat(x, y)];
        return 0ull;
    }

    struct ZInit {
        ZInit()
        {
            std::mt19937_64 rng(0x9E3779B97F4A7C15ULL); // seed fixe (reproductible)
            for (auto& v : Z_PCS)
                v = rng();
            Z_SIDE = rng();
        }
    } ZINIT;
}
// ------------------------------------------------

Board::Board() { reset(); }

Cell Board::at(uint8_t x, uint8_t y) const
{
    if (!isInside(x, y))
        return Cell::Empty;
    return cells[idx(x, y)];
}

void Board::reset()
{
    cells.fill(Cell::Empty);
    currentPlayer = Player::Black;
    blackPairs = whitePairs = 0;
    gameState = GameStatus::Ongoing;
    moveHistory.clear();

    // Zobrist
    zobristHash = 0ull;
    // Encode le trait (Black to move)
    zobristHash ^= Z_SIDE;
}

// ------------------------------------------------
// Double-trois (free-threes) avec prise en compte des captures
bool Board::createsIllegalDoubleThree(Move m, const RuleSet& rules) const
{
    if (!rules.forbidDoubleThree)
        return false;

    // Exception: un coup QUI CAPTURE est autorisé même s'il crée un double-trois
    if (rules.capturesEnabled && wouldCapture(m))
        return false;

    const Cell ME = playerToCell(m.by);
    const Cell OP = (ME == Cell::Black ? Cell::White : Cell::Black);

    // cases virtuellement retirées par capture causée par m
    auto capturedVirt = [&](int x, int y) -> bool {
        static constexpr int DX[4] = { 1, 0, 1, 1 };
        static constexpr int DY[4] = { 0, 1, 1, -1 };
        for (int d = 0; d < 4; ++d) {
            // sens +
            int x1 = m.pos.x + DX[d], y1 = m.pos.y + DY[d];
            int x2 = m.pos.x + 2 * DX[d], y2 = m.pos.y + 2 * DY[d];
            int x3 = m.pos.x + 3 * DX[d], y3 = m.pos.y + 3 * DY[d];
            if (isInside(x3, y3) && at(x1, y1) == OP && at(x2, y2) == OP && at(x3, y3) == ME)
                return (x == x1 && y == y1) || (x == x2 && y == y2);
            // sens -
            int X1 = m.pos.x - DX[d], Y1 = m.pos.y - DY[d];
            int X2 = m.pos.x - 2 * DX[d], Y2 = m.pos.y - 2 * DY[d];
            int X3 = m.pos.x - 3 * DX[d], Y3 = m.pos.y - 3 * DY[d];
            if (isInside(X3, Y3) && at(X1, Y1) == OP && at(X2, Y2) == OP && at(X3, Y3) == ME)
                return (x == X1 && y == Y1) || (x == X2 && y == Y2);
        }
        return false;
    };

    auto vAt = [&](int x, int y) -> Cell {
        if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE)
            return OP; // mur = adversaire
        if ((int)m.pos.x == x && (int)m.pos.y == y)
            return ME;
        if (capturedVirt(x, y))
            return Cell::Empty;
        return at(x, y);
    };

    auto hasThreeInLine = [&](int dx, int dy) -> bool {
        std::string s;
        s.reserve(11);
        for (int k = -5; k <= 5; ++k) {
            int x = (int)m.pos.x + k * dx;
            int y = (int)m.pos.y + k * dy;
            Cell c = vAt(x, y);
            char ch = (c == Cell::Empty) ? '0' : (c == ME ? '1' : '2');
            s.push_back(ch);
        }
        auto contains = [&](const std::string& pat) -> bool {
            return s.find(pat) != std::string::npos;
        };
        if (contains("01110"))
            return true; // 0 111 0
        if (contains("010110"))
            return true; // 0 1 0 11 0
        if (contains("011010"))
            return true; // 0 11 0 1 0
        return false;
    };

    int threes = 0;
    if (hasThreeInLine(1, 0))
        ++threes;
    if (hasThreeInLine(0, 1))
        ++threes;
    if (hasThreeInLine(1, 1))
        ++threes;
    if (hasThreeInLine(1, -1))
        ++threes;

    return threes >= 2;
}

// ------------------------------------------------
// Détecte 5+ alignés depuis p (8 directions)
bool Board::checkFiveOrMoreFrom(Pos p, Cell who) const
{
    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };
    for (int d = 0; d < 4; ++d) {
        int count = 1;
        for (int s = -1; s <= 1; s += 2) {
            int x = p.x, y = p.y;
            while (true) {
                x += s * DX[d];
                y += s * DY[d];
                if (!isInside(x, y))
                    break;
                if (at(x, y) == who)
                    ++count;
                else
                    break;
            }
        }
        if (count >= 5)
            return true;
    }
    return false;
}

// ------------------------------------------------
// Captures XOOX dans 4 directions et 2 sens
int Board::applyCapturesAround(Pos p, Cell who, const RuleSet& rules, std::vector<Pos>& removed)
{
    if (!rules.capturesEnabled)
        return 0;

    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    const Cell opp = (who == Cell::Black ? Cell::White : Cell::Black);
    int pairs = 0;

    auto tryDir = [&](int sx, int sy, int dx, int dy) -> bool {
        int x1 = sx + dx, y1 = sy + dy;
        int x2 = sx + 2 * dx, y2 = sy + 2 * dy;
        int x3 = sx + 3 * dx, y3 = sy + 3 * dy;
        if (!isInside(x3, y3))
            return false;
        if (at(x1, y1) == opp && at(x2, y2) == opp && at(x3, y3) == who) {
            cells[idx(x1, y1)] = Cell::Empty;
            cells[idx(x2, y2)] = Cell::Empty;
            removed.push_back({ (uint8_t)x1, (uint8_t)y1 });
            removed.push_back({ (uint8_t)x2, (uint8_t)y2 });
            return true;
        }
        return false;
    };

    for (int d = 0; d < 4; ++d) {
        if (tryDir(p.x, p.y, DX[d], DY[d]))
            ++pairs; // sens +
        if (tryDir(p.x, p.y, -DX[d], -DY[d]))
            ++pairs; // sens -
    }
    return pairs;
}

// ------------------------------------------------
PlayResult Board::tryPlay(Move m, const RuleSet& rules)
{
    if (gameState != GameStatus::Ongoing) {
        return PlayResult::fail(PlayErrorCode::GameFinished, "Game already finished.");
    }
    if (m.by != currentPlayer) {
        return PlayResult::fail(PlayErrorCode::NotPlayersTurn, "Not this player's turn.");
    }
    if (!isEmpty(m.pos.x, m.pos.y)) {
        return PlayResult::fail(PlayErrorCode::Occupied, "Cell not empty.");
    }

    // Must-break rule: if opponent (justPlayed) currently has a breakable 5+, the side to move
    // must capture to break it (or win by capture). Non-breaking moves are illegal.
    bool mustBreak = false;
    if (rules.allowFiveOrMore && rules.capturesEnabled) {
        Player justPlayed = opponent(currentPlayer);
        Cell meC = playerToCell(justPlayed);
        if (hasAnyFive(meC) && isFiveBreakableNow(justPlayed, rules))
            mustBreak = true;
    }

    bool allowDoubleThreeThisMove = false;
    if (mustBreak) {
        // Only capturing moves can break a 5+; quickly reject otherwise
        if (!wouldCapture(m)) {
            return PlayResult::fail(PlayErrorCode::RuleViolation, "Must break opponent's five.");
        }
        // Simulate capture effect to ensure this move actually breaks (or wins by capture)
        Board sim = *this;
        // place the stone (ignore pattern illegality here since must-break allows capture exceptions)
        sim.cells[idx(m.pos.x, m.pos.y)] = playerToCell(m.by);
        std::vector<Pos> removedTmp;
        int gainedTmp = sim.applyCapturesAround(m.pos, playerToCell(m.by), rules, removedTmp);
        if (gainedTmp) {
            if (m.by == Player::Black)
                sim.blackPairs += gainedTmp;
            else
                sim.whitePairs += gainedTmp;
        }
        int myPairsAfter = (m.by == Player::Black ? sim.blackPairs : sim.whitePairs);
        Cell oppFiveColor = playerToCell(opponent(currentPlayer));
        bool breaks = (myPairsAfter >= rules.captureWinPairs) || (!sim.hasAnyFive(oppFiveColor));
        if (!breaks) {
            return PlayResult::fail(PlayErrorCode::RuleViolation, "Must break opponent's five.");
        }
        // This capture move is allowed even if it forms a double-three pattern
        allowDoubleThreeThisMove = true;
    }

    // Double-three rule, unless allowed due to must-break capture
    if (!allowDoubleThreeThisMove && createsIllegalDoubleThree(m, rules)) {
        return PlayResult::fail(PlayErrorCode::RuleViolation, "Illegal double-three.");
    }

    // Enregistrer état pour undo
    UndoEntry u;
    u.move = m;
    u.blackPairsBefore = blackPairs;
    u.whitePairsBefore = whitePairs;
    u.stateBefore = gameState;
    u.playerBefore = currentPlayer;

    // Placer la pierre
    cells[idx(m.pos.x, m.pos.y)] = playerToCell(m.by);
    // Zobrist: ajouter la pierre
    zobristHash ^= z_of(playerToCell(m.by), m.pos.x, m.pos.y);

    // Captures (XOOX)
    int gained = applyCapturesAround(m.pos, playerToCell(m.by), rules, u.capturedStones);
    if (gained) {
        if (m.by == Player::Black)
            blackPairs += gained;
        else
            whitePairs += gained;
        // Zobrist: retirer les capturées (couleur adverse)
        Cell oppC = (m.by == Player::Black ? Cell::White : Cell::Black);
        for (auto rp : u.capturedStones) {
            zobristHash ^= z_of(oppC, rp.x, rp.y);
        }
    }

    // Victoire par 5+ avec nuance "cassable par capture"
    if (rules.allowFiveOrMore && checkFiveOrMoreFrom(m.pos, playerToCell(m.by))) {
        if (!isFiveBreakableNow(m.by, rules)) {
            gameState = GameStatus::WinByAlign;
        }
    }

    // Victoire par captures (ne pas écraser une victoire déjà établie si on veut priorité align)
    if (rules.capturesEnabled && gameState == GameStatus::Ongoing) {
        if (blackPairs >= rules.captureWinPairs || whitePairs >= rules.captureWinPairs)
            gameState = GameStatus::WinByCapture;
    }

    // Nulle: plateau plein sans victoire
    if (gameState == GameStatus::Ongoing && isBoardFull())
        gameState = GameStatus::Draw;

    moveHistory.push_back(std::move(u));
    currentPlayer = opponent(currentPlayer);
    // Zobrist: side-to-move
    zobristHash ^= Z_SIDE;

    return PlayResult::ok();
}

// ------------------------------------------------
bool Board::undo()
{
    if (moveHistory.empty())
        return false;
    UndoEntry u = std::move(moveHistory.back());
    moveHistory.pop_back();

    // Zobrist: le trait redevient celui d'avant
    zobristHash ^= Z_SIDE;

    // Retirer la pierre jouée
    cells[idx(u.move.pos.x, u.move.pos.y)] = Cell::Empty;
    // Zobrist: retirer la pierre annulée
    zobristHash ^= z_of(playerToCell(u.move.by), u.move.pos.x, u.move.pos.y);

    // Restaurer les pierres capturées
    Cell oppC = (u.move.by == Player::Black ? Cell::White : Cell::Black);
    for (auto rp : u.capturedStones) {
        cells[idx(rp.x, rp.y)] = oppC;
        // Zobrist: remettre les capturées
        zobristHash ^= z_of(oppC, rp.x, rp.y);
    }
    blackPairs = u.blackPairsBefore;
    whitePairs = u.whitePairsBefore;
    gameState = u.stateBefore;
    currentPlayer = u.playerBefore;
    return true;
}

// ------------------------------------------------
std::vector<Move> Board::legalMoves(Player p, const RuleSet& rules) const
{
    std::vector<Move> out;

    out.reserve(BOARD_SIZE * BOARD_SIZE);
    for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
        for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
            if (at(x, y) != Cell::Empty)
                continue;

            // Fenêtrage simple: garder cases proches d'une pierre existante
            bool near = false;
            for (int dy = -2; dy <= 2 && !near; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (!isInside(nx, ny))
                        continue;
                    if (at(nx, ny) != Cell::Empty) {
                        near = true;
                        break;
                    }
                }
            }
            if (!near && !moveHistory.empty())
                continue;

            Move m { { x, y }, p };
            if (createsIllegalDoubleThree(m, rules))
                continue;

            out.push_back(m);
        }
    }
    return out;
}

// ------------------------------------------------
bool Board::hasAnyFive(Cell who) const
{
    for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
        for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
            if (at(x, y) == who) {
                if (checkFiveOrMoreFrom({ x, y }, who))
                    return true;
            }
        }
    }
    return false;
}

// Après que 'justPlayed' a posé sa pierre et que les captures ont été appliquées,
// vérifier si l'adversaire peut casser immédiatement le 5+ par capture
bool Board::isFiveBreakableNow(Player justPlayed, const RuleSet& rules) const
{
    if (!rules.capturesEnabled)
        return false;

    Player opp = (justPlayed == Player::Black ? Player::White : Player::Black);
    Cell meC = playerToCell(justPlayed);

    Board base = *this;
    base.forceSide(opp); // au trait pour l'adversaire dans la simulation

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (base.at(x, y) != Cell::Empty)
                continue;

            Move mv { Pos { (uint8_t)x, (uint8_t)y }, opp };

            if (!base.wouldCapture(mv))
                continue; // only capturing moves can break immediately

            Board sim = base;
            // place the stone and apply captures
            sim.cells[idx(mv.pos.x, mv.pos.y)] = playerToCell(opp);
            std::vector<Pos> removed;
            int gained = sim.applyCapturesAround(mv.pos, playerToCell(opp), rules, removed);
            if (gained) {
                if (opp == Player::Black)
                    sim.blackPairs += gained;
                else
                    sim.whitePairs += gained;
            }

            int oppPairsAfter = (opp == Player::Black ? sim.blackPairs : sim.whitePairs);
            if (oppPairsAfter >= rules.captureWinPairs)
                return true; // immediate win by capture
            if (!sim.hasAnyFive(meC))
                return true; // line is broken by the capture
        }
    }
    return false;
}

void Board::forceSide(Player p)
{
    if (currentPlayer != p) {
        currentPlayer = p;
        // Maintenir la clé Zobrist alignée avec "side to move"
        zobristHash ^= Z_SIDE;
    }
}

bool Board::isBoardFull() const
{
    for (const auto& c : cells)
        if (c == Cell::Empty)
            return false;
    return true;
}

// Détecte si m provoquerait une capture XOOX (±4 directions)
bool Board::wouldCapture(Move m) const
{
    const Cell me = playerToCell(m.by);
    const Cell opp = (me == Cell::Black ? Cell::White : Cell::Black);
    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };

    auto inside = [&](int X, int Y) { return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

    for (int d = 0; d < 4; ++d) {
        int x1 = m.pos.x + DX[d], y1 = m.pos.y + DY[d];
        int x2 = m.pos.x + 2 * DX[d], y2 = m.pos.y + 2 * DY[d];
        int x3 = m.pos.x + 3 * DX[d], y3 = m.pos.y + 3 * DY[d];
        if (inside(x1, y1) && inside(x2, y2) && inside(x3, y3) && at(x1, y1) == opp && at(x2, y2) == opp && at(x3, y3) == me)
            return true;

        int X1 = m.pos.x - DX[d], Y1 = m.pos.y - DY[d];
        int X2 = m.pos.x - 2 * DX[d], Y2 = m.pos.y - 2 * DY[d];
        int X3 = m.pos.x - 3 * DX[d], Y3 = m.pos.y - 3 * DY[d];
        if (inside(X1, Y1) && inside(X2, Y2) && inside(X3, Y3) && at(X1, Y1) == opp && at(X2, Y2) == opp && at(X3, Y3) == me)
            return true;
    }
    return false;
}

} // namespace gomoku
