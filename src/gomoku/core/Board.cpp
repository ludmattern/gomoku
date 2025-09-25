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
    blackStones = whiteStones = 0;
    gameState = GameStatus::Ongoing;
    moveHistory.clear();
    occupied_.clear();
    occIdx_.fill(-1);

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
            if (isInside(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) && at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == OP && at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == OP && at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == ME)
                return (x == x1 && y == y1) || (x == x2 && y == y2);
            // sens -
            int X1 = m.pos.x - DX[d], Y1 = m.pos.y - DY[d];
            int X2 = m.pos.x - 2 * DX[d], Y2 = m.pos.y - 2 * DY[d];
            int X3 = m.pos.x - 3 * DX[d], Y3 = m.pos.y - 3 * DY[d];
            if (isInside(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) && at(static_cast<uint8_t>(X1), static_cast<uint8_t>(Y1)) == OP && at(static_cast<uint8_t>(X2), static_cast<uint8_t>(Y2)) == OP && at(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) == ME)
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
        return at(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
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
                if (!isInside(static_cast<uint8_t>(x), static_cast<uint8_t>(y)))
                    break;
                if (at(static_cast<uint8_t>(x), static_cast<uint8_t>(y)) == who)
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
        if (!isInside(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)))
            return false;
        if (at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == opp && at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == opp && at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == who) {
            cells[idx(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1))] = Cell::Empty;
            cells[idx(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2))] = Cell::Empty;
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
PlayResult Board::applyCore(Move m, const RuleSet& rules, bool record)
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

    bool mustBreak = false;
    if (rules.allowFiveOrMore && rules.capturesEnabled) {
        Player justPlayed = opponent(currentPlayer);
        Cell meC = playerToCell(justPlayed);
        if (hasAnyFive(meC) && isFiveBreakableNow(justPlayed, rules))
            mustBreak = true;
    }

    bool allowDoubleThreeThisMove = false;
    if (mustBreak) {
        if (!wouldCapture(m)) {
            return PlayResult::fail(PlayErrorCode::RuleViolation, "Must break opponent's five.");
        }
        Board sim = *this;
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
        allowDoubleThreeThisMove = true;
    }

    if (!allowDoubleThreeThisMove && createsIllegalDoubleThree(m, rules)) {
        return PlayResult::fail(PlayErrorCode::RuleViolation, "Illegal double-three.");
    }

    // Préparation Undo (si record)
    UndoEntry u;
    if (record) {
        u.move = m;
        u.blackPairsBefore = blackPairs;
        u.whitePairsBefore = whitePairs;
        u.blackStonesBefore = blackStones;
        u.whiteStonesBefore = whiteStones;
        u.stateBefore = gameState;
        u.playerBefore = currentPlayer;
    }

    cells[idx(m.pos.x, m.pos.y)] = playerToCell(m.by);
    zobristHash ^= z_of(playerToCell(m.by), m.pos.x, m.pos.y);
    // Track stone counts
    if (m.by == Player::Black)
        ++blackStones;
    else
        ++whiteStones;
    // Sparse index add
    {
        const int id = idx(m.pos.x, m.pos.y);
        occIdx_[id] = static_cast<int16_t>(occupied_.size());
        occupied_.push_back(m.pos);
    }

    std::vector<Pos> capturedLocal; // utilisera u.capturedStones si record
    auto& capVec = record ? u.capturedStones : capturedLocal;
    int gained = applyCapturesAround(m.pos, playerToCell(m.by), rules, capVec);
    if (gained) {
        if (m.by == Player::Black)
            blackPairs += gained;
        else
            whitePairs += gained;
        Cell oppC = (m.by == Player::Black ? Cell::White : Cell::Black);
        for (auto rp : capVec) {
            zobristHash ^= z_of(oppC, rp.x, rp.y);
            // Decrement opponent stone counts for captured stones
            if (oppC == Cell::Black)
                --blackStones;
            else
                --whiteStones;
            // Sparse index remove via swap-pop
            const int id = idx(rp.x, rp.y);
            int16_t posIdx = occIdx_[id];
            if (posIdx >= 0) {
                const int lastIdx = static_cast<int>(occupied_.size()) - 1;
                if (posIdx != lastIdx) {
                    Pos moved = occupied_.back();
                    occupied_[posIdx] = moved;
                    occIdx_[moved.toIndex()] = posIdx;
                }
                occupied_.pop_back();
                occIdx_[id] = -1;
            }
        }
    }

    if (rules.allowFiveOrMore && checkFiveOrMoreFrom(m.pos, playerToCell(m.by))) {
        if (!isFiveBreakableNow(m.by, rules)) {
            gameState = GameStatus::WinByAlign;
        }
    }

    if (rules.capturesEnabled && gameState == GameStatus::Ongoing) {
        if (blackPairs >= rules.captureWinPairs || whitePairs >= rules.captureWinPairs)
            gameState = GameStatus::WinByCapture;
    }

    if (gameState == GameStatus::Ongoing && isBoardFull())
        gameState = GameStatus::Draw;

    if (record) {
        moveHistory.push_back(std::move(u));
    }
    currentPlayer = opponent(currentPlayer);
    zobristHash ^= Z_SIDE;

    return PlayResult::ok();
}

PlayResult Board::tryPlay(Move m, const RuleSet& rules)
{
    return applyCore(m, rules, true);
}

bool Board::speculativeTry(Move m, const RuleSet& rules, PlayResult* out)
{
    // Implémentation Option B (diff ciblé) :
    // On enregistre uniquement l'état minimal nécessaire pour restaurer :
    // - hash, joueur courant, compteurs captures, statut
    // - contenu des cellules susceptibles de changer (la case jouée + jusqu'à 8*2 candidates captures)

    uint64_t hashBefore = zobristHash;
    Player playerBefore = currentPlayer;
    int blackPairsBefore = blackPairs;
    int whitePairsBefore = whitePairs;
    int blackStonesBefore = blackStones;
    int whiteStonesBefore = whiteStones;
    GameStatus statusBefore = gameState;

    struct CellSnapshot {
        Pos p;
        Cell before;
    };
    CellSnapshot center { m.pos, at(m.pos.x, m.pos.y) }; // devrait être Empty si légal

    // Générer les positions candidates pouvant être capturées (x1,x2) pour chaque direction ±
    static constexpr int DX[4] = { 1, 0, 1, 1 };
    static constexpr int DY[4] = { 0, 1, 1, -1 };
    CellSnapshot candidates[16];
    int candCount = 0;
    bool seen[BOARD_SIZE * BOARD_SIZE] = { false };
    auto mark = [&](int x, int y) {
        if (!isInside(static_cast<uint8_t>(x), static_cast<uint8_t>(y)))
            return;
        int idFlat = y * BOARD_SIZE + x;
        if (seen[idFlat])
            return;
        seen[idFlat] = true;
        candidates[candCount++] = CellSnapshot { Pos { (uint8_t)x, (uint8_t)y }, at(static_cast<uint8_t>(x), static_cast<uint8_t>(y)) };
    };
    for (int d = 0; d < 4; ++d) {
        int x1 = m.pos.x + DX[d], y1 = m.pos.y + DY[d];
        int x2 = m.pos.x + 2 * DX[d], y2 = m.pos.y + 2 * DY[d];
        mark(x1, y1);
        mark(x2, y2);
        int X1 = m.pos.x - DX[d], Y1 = m.pos.y - DY[d];
        int X2 = m.pos.x - 2 * DX[d], Y2 = m.pos.y - 2 * DY[d];
        mark(X1, Y1);
        mark(X2, Y2);
    }

    PlayResult pr = applyCore(m, rules, false);
    if (out)
        *out = pr;
    if (!pr.success) {
        // Aucune mutation durable si échec (toutes les validations échouantes précèdent la pose).
        return false;
    }

    // ROLLBACK : retirer la pierre posée et restaurer les cellules capturées.
    // La pierre jouée est toujours à m.pos si succès.
    cells[idx(m.pos.x, m.pos.y)] = center.before; // normalement Empty
    // adjust stone counts for the removed placed stone
    if (m.by == Player::Black)
        --blackStones;
    else
        --whiteStones;

    // Restaurer chaque cellule candidate devenue vide alors qu'elle ne l'était pas avant
    for (int i = 0; i < candCount; ++i) {
        auto& snap = candidates[i];
        Cell after = at(snap.p.x, snap.p.y);
        // Si la cellule a été vidée (capture) on la restaure.
        if (snap.before != Cell::Empty && after == Cell::Empty) {
            // (snap.before devrait être oppC en pratique)
            cells[idx(snap.p.x, snap.p.y)] = snap.before;
            if (snap.before == Cell::Black)
                ++blackStones;
            else if (snap.before == Cell::White)
                ++whiteStones;
        }
    }

    // Restaurer compteurs & statut & trait & hash
    blackPairs = blackPairsBefore;
    whitePairs = whitePairsBefore;
    blackStones = blackStonesBefore;
    whiteStones = whiteStonesBefore;
    gameState = statusBefore;
    currentPlayer = playerBefore;
    zobristHash = hashBefore; // hash global cohérent (inclut side to move)

    return true;
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
    // Update stone count for removed stone
    if (u.move.by == Player::Black)
        --blackStones;
    else
        --whiteStones;
    // Sparse index remove via swap-pop
    {
        const int id = idx(u.move.pos.x, u.move.pos.y);
        int16_t posIdx = occIdx_[id];
        if (posIdx >= 0) {
            const int lastIdx = static_cast<int>(occupied_.size()) - 1;
            if (posIdx != lastIdx) {
                Pos moved = occupied_.back();
                occupied_[posIdx] = moved;
                occIdx_[moved.toIndex()] = posIdx;
            }
            occupied_.pop_back();
            occIdx_[id] = -1;
        }
    }

    // Restaurer les pierres capturées
    Cell oppC = (u.move.by == Player::Black ? Cell::White : Cell::Black);
    for (auto rp : u.capturedStones) {
        cells[idx(rp.x, rp.y)] = oppC;
        // Zobrist: remettre les capturées
        zobristHash ^= z_of(oppC, rp.x, rp.y);
        if (oppC == Cell::Black)
            ++blackStones;
        else
            ++whiteStones;
        // Sparse index add back
        const int id = idx(rp.x, rp.y);
        occIdx_[id] = static_cast<int16_t>(occupied_.size());
        occupied_.push_back(rp);
    }
    blackPairs = u.blackPairsBefore;
    whitePairs = u.whitePairsBefore;
    blackStones = u.blackStonesBefore;
    whiteStones = u.whiteStonesBefore;
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
                    if (!isInside(static_cast<uint8_t>(nx), static_cast<uint8_t>(ny)))
                        continue;
                    if (at(static_cast<uint8_t>(nx), static_cast<uint8_t>(ny)) != Cell::Empty) {
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
            if (base.at(static_cast<uint8_t>(x), static_cast<uint8_t>(y)) != Cell::Empty)
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
        if (inside(x1, y1) && inside(x2, y2) && inside(x3, y3) && at(static_cast<uint8_t>(x1), static_cast<uint8_t>(y1)) == opp && at(static_cast<uint8_t>(x2), static_cast<uint8_t>(y2)) == opp && at(static_cast<uint8_t>(x3), static_cast<uint8_t>(y3)) == me)
            return true;

        int X1 = m.pos.x - DX[d], Y1 = m.pos.y - DY[d];
        int X2 = m.pos.x - 2 * DX[d], Y2 = m.pos.y - 2 * DY[d];
        int X3 = m.pos.x - 3 * DX[d], Y3 = m.pos.y - 3 * DY[d];
        if (inside(X1, Y1) && inside(X2, Y2) && inside(X3, Y3) && at(static_cast<uint8_t>(X1), static_cast<uint8_t>(Y1)) == opp && at(static_cast<uint8_t>(X2), static_cast<uint8_t>(Y2)) == opp && at(static_cast<uint8_t>(X3), static_cast<uint8_t>(Y3)) == me)
            return true;
    }
    return false;
}

} // namespace gomoku
