#include "Board.hpp"
#include <cassert>
#include <array>
#include <random>
#include <string>

namespace gomoku
{

	static inline Cell cellOf(Player p) { return p == Player::Black ? Cell::Black : Cell::White; }
	static inline Player other(Player p) { return p == Player::Black ? Player::White : Player::Black; }

	// ------------------ Zobrist ------------------
	namespace
	{
		std::array<uint64_t, 2 * BOARD_SIZE * BOARD_SIZE> Z_PCS{};
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

		struct ZInit
		{
			ZInit()
			{
				std::mt19937_64 rng(0x9E3779B97F4A7C15ULL); // seed fixe (reproductible)
				for (auto &v : Z_PCS)
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
		side = Player::Black;
		blackPairs = whitePairs = 0;
		state = GameStatus::Ongoing;
		history.clear();

		// Zobrist
		zobrist_ = 0ull;
		// Encode le trait (Black to move)
		zobrist_ ^= Z_SIDE;
	}

	// ------------------------------------------------
	// Double-trois (free-threes) avec prise en compte des captures
	bool Board::createsIllegalDoubleThree(Move m, const RuleSet &rules) const
	{
		if (!rules.forbidDoubleThree)
			return false;

		// Exception: un coup QUI CAPTURE est autorisé même s'il crée un double-trois
		if (rules.capturesEnabled && wouldCapture(m))
			return false;

		const Cell ME = cellOf(m.by);
		const Cell OP = (ME == Cell::Black ? Cell::White : Cell::Black);

		// cases virtuellement retirées par capture causée par m
		auto capturedVirt = [&](int x, int y) -> bool
		{
			static constexpr int DX[4] = {1, 0, 1, 1};
			static constexpr int DY[4] = {0, 1, 1, -1};
			for (int d = 0; d < 4; ++d)
			{
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

		auto vAt = [&](int x, int y) -> Cell
		{
			if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE)
				return OP; // mur = adversaire
			if ((int)m.pos.x == x && (int)m.pos.y == y)
				return ME;
			if (capturedVirt(x, y))
				return Cell::Empty;
			return at(x, y);
		};

		auto hasThreeInLine = [&](int dx, int dy) -> bool
		{
			std::string s;
			s.reserve(11);
			for (int k = -5; k <= 5; ++k)
			{
				int x = (int)m.pos.x + k * dx;
				int y = (int)m.pos.y + k * dy;
				Cell c = vAt(x, y);
				char ch = (c == Cell::Empty) ? '0' : (c == ME ? '1' : '2');
				s.push_back(ch);
			}
			auto contains = [&](const std::string &pat) -> bool
			{
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
		static constexpr int DX[4] = {1, 0, 1, 1};
		static constexpr int DY[4] = {0, 1, 1, -1};
		for (int d = 0; d < 4; ++d)
		{
			int count = 1;
			for (int s = -1; s <= 1; s += 2)
			{
				int x = p.x, y = p.y;
				while (true)
				{
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
	int Board::applyCapturesAround(Pos p, Cell who, const RuleSet &rules, std::vector<Pos> &removed)
	{
		if (!rules.capturesEnabled)
			return 0;

		static constexpr int DX[4] = {1, 0, 1, 1};
		static constexpr int DY[4] = {0, 1, 1, -1};

		const Cell opp = (who == Cell::Black ? Cell::White : Cell::Black);
		int pairs = 0;

		auto tryDir = [&](int sx, int sy, int dx, int dy) -> bool
		{
			int x1 = sx + dx, y1 = sy + dy;
			int x2 = sx + 2 * dx, y2 = sy + 2 * dy;
			int x3 = sx + 3 * dx, y3 = sy + 3 * dy;
			if (!isInside(x3, y3))
				return false;
			if (at(x1, y1) == opp && at(x2, y2) == opp && at(x3, y3) == who)
			{
				cells[idx(x1, y1)] = Cell::Empty;
				cells[idx(x2, y2)] = Cell::Empty;
				removed.push_back({(uint8_t)x1, (uint8_t)y1});
				removed.push_back({(uint8_t)x2, (uint8_t)y2});
				return true;
			}
			return false;
		};

		for (int d = 0; d < 4; ++d)
		{
			if (tryDir(p.x, p.y, DX[d], DY[d]))
				++pairs; // sens +
			if (tryDir(p.x, p.y, -DX[d], -DY[d]))
				++pairs; // sens -
		}
		return pairs;
	}

	// ------------------------------------------------
	bool Board::play(Move m, const RuleSet &rules, std::string *whyNot)
	{
		if (state != GameStatus::Ongoing)
		{
			if (whyNot)
				*whyNot = "Game already finished.";
			return false;
		}
		if (m.by != side)
		{
			if (whyNot)
				*whyNot = "Not this player's turn.";
			return false;
		}
		if (!isEmpty(m.pos.x, m.pos.y))
		{
			if (whyNot)
				*whyNot = "Cell not empty.";
			return false;
		}
		if (createsIllegalDoubleThree(m, rules))
		{
			if (whyNot)
				*whyNot = "Illegal double-three.";
			return false;
		}

		// Enregistrer état pour undo
		UndoEntry u;
		u.move = m;
		u.blackPairsBefore = blackPairs;
		u.whitePairsBefore = whitePairs;
		u.stateBefore = state;
		u.sideBefore = side;

		// Placer la pierre
		cells[idx(m.pos.x, m.pos.y)] = cellOf(m.by);
		// Zobrist: ajouter la pierre
		zobrist_ ^= z_of(cellOf(m.by), m.pos.x, m.pos.y);

		// Captures (XOOX)
		int gained = applyCapturesAround(m.pos, cellOf(m.by), rules, u.removed);
		if (gained)
		{
			if (m.by == Player::Black)
				blackPairs += gained;
			else
				whitePairs += gained;
			// Zobrist: retirer les capturées (couleur adverse)
			Cell oppC = (m.by == Player::Black ? Cell::White : Cell::Black);
			for (auto rp : u.removed)
			{
				zobrist_ ^= z_of(oppC, rp.x, rp.y);
			}
		}

		// Victoire par 5+ avec nuance "cassable par capture"
		if (rules.allowFiveOrMore && checkFiveOrMoreFrom(m.pos, cellOf(m.by)))
		{
			if (!isFiveBreakableNow(m.by, rules))
			{
				state = GameStatus::WinByAlign;
			}
		}

		// Victoire par captures (ne pas écraser une victoire déjà établie si on veut priorité align)
		if (rules.capturesEnabled && state == GameStatus::Ongoing)
		{
			if (blackPairs >= rules.captureWinPairs || whitePairs >= rules.captureWinPairs)
				state = GameStatus::WinByCapture;
		}

		// Nulle: plateau plein sans victoire
		if (state == GameStatus::Ongoing && isBoardFull())
			state = GameStatus::Draw;

		history.push_back(std::move(u));
		side = other(side);
		// Zobrist: side-to-move
		zobrist_ ^= Z_SIDE;

		return true;
	}

	// ------------------------------------------------
	bool Board::undo()
	{
		if (history.empty())
			return false;
		UndoEntry u = std::move(history.back());
		history.pop_back();

		// Zobrist: le trait redevient celui d'avant
		zobrist_ ^= Z_SIDE;

		// Retirer la pierre jouée
		cells[idx(u.move.pos.x, u.move.pos.y)] = Cell::Empty;
		// Zobrist: retirer la pierre annulée
		zobrist_ ^= z_of(cellOf(u.move.by), u.move.pos.x, u.move.pos.y);

		// Restaurer les pierres capturées
		Cell oppC = (u.move.by == Player::Black ? Cell::White : Cell::Black);
		for (auto rp : u.removed)
		{
			cells[idx(rp.x, rp.y)] = oppC;
			// Zobrist: remettre les capturées
			zobrist_ ^= z_of(oppC, rp.x, rp.y);
		}
		blackPairs = u.blackPairsBefore;
		whitePairs = u.whitePairsBefore;
		state = u.stateBefore;
		side = u.sideBefore;
		return true;
	}

	// ------------------------------------------------
	std::vector<Move> Board::legalMoves(Player p, const RuleSet &rules) const
	{
		std::vector<Move> out;

		out.reserve(BOARD_SIZE * BOARD_SIZE);
		for (uint8_t y = 0; y < BOARD_SIZE; ++y)
		{
			for (uint8_t x = 0; x < BOARD_SIZE; ++x)
			{
				if (at(x, y) != Cell::Empty)
					continue;

				// Fenêtrage simple: garder cases proches d'une pierre existante
				bool near = false;
				for (int dy = -2; dy <= 2 && !near; ++dy)
				{
					for (int dx = -2; dx <= 2; ++dx)
					{
						int nx = x + dx, ny = y + dy;
						if (!isInside(nx, ny))
							continue;
						if (at(nx, ny) != Cell::Empty)
						{
							near = true;
							break;
						}
					}
				}
				if (!near && !history.empty())
					continue;

				Move m{{x, y}, p};
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
		for (uint8_t y = 0; y < BOARD_SIZE; ++y)
		{
			for (uint8_t x = 0; x < BOARD_SIZE; ++x)
			{
				if (at(x, y) == who)
				{
					if (checkFiveOrMoreFrom({x, y}, who))
						return true;
				}
			}
		}
		return false;
	}

	// Après que 'justPlayed' a posé sa pierre et que les captures ont été appliquées,
	// vérifier si l'adversaire peut casser immédiatement le 5+ par capture
	bool Board::isFiveBreakableNow(Player justPlayed, const RuleSet &rules) const
	{
		if (!rules.capturesEnabled)
			return false;

		Player opp = (justPlayed == Player::Black ? Player::White : Player::Black);
		Cell meC = cellOf(justPlayed);

		Board base = *this;
		base.forceSide(opp); // au trait pour l'adversaire dans la simulation

		for (int y = 0; y < BOARD_SIZE; ++y)
		{
			for (int x = 0; x < BOARD_SIZE; ++x)
			{
				if (base.at(x, y) != Cell::Empty)
					continue;

				Move mv{Pos{(uint8_t)x, (uint8_t)y}, opp};

				// 1) voie normale : on laisse play() décider (captures + règles)
				Board sim = base;
				std::string why;
				if (sim.play(mv, rules, &why))
				{
					auto [bp, wp] = sim.capturedPairs();
					int oppPairs = (opp == Player::Black ? bp : wp);
					if (oppPairs >= rules.captureWinPairs)
						return true; // l’adversaire gagnerait par captures
					if (!sim.hasAnyFive(meC))
						return true; // la ligne de 5 de justPlayed est cassée
					continue;
				}

				// 2) fallback : refusé pour double-trois mais le coup CAPTURE -> autorisé par la règle
				if (why == "Illegal double-three." && base.wouldCapture(mv))
				{
					Board sim2 = base;

					// poser la pierre adverse
					sim2.cells[idx(mv.pos.x, mv.pos.y)] = cellOf(opp);

					// appliquer les captures (comme play)
					std::vector<Pos> removed;
					int gained = sim2.applyCapturesAround(mv.pos, cellOf(opp), rules, removed);
					if (gained)
					{
						if (opp == Player::Black)
							sim2.blackPairs += gained;
						else
							sim2.whitePairs += gained;
					}

					int oppPairs2 = (opp == Player::Black ? sim2.blackPairs : sim2.whitePairs);
					if (oppPairs2 >= rules.captureWinPairs)
						return true;
					if (!sim2.hasAnyFive(meC))
						return true;
				}
			}
		}
		return false;
	}

	void Board::forceSide(Player p)
	{
		if (side != p)
		{
			side = p;
			// Maintenir la clé Zobrist alignée avec "side to move"
			zobrist_ ^= Z_SIDE;
		}
	}

	bool Board::isBoardFull() const
	{
		for (const auto &c : cells)
			if (c == Cell::Empty)
				return false;
		return true;
	}

	// Détecte si m provoquerait une capture XOOX (±4 directions)
	bool Board::wouldCapture(Move m) const
	{
		const Cell me = cellOf(m.by);
		const Cell opp = (me == Cell::Black ? Cell::White : Cell::Black);
		static constexpr int DX[4] = {1, 0, 1, 1};
		static constexpr int DY[4] = {0, 1, 1, -1};

		auto inside = [&](int X, int Y)
		{ return 0 <= X && X < BOARD_SIZE && 0 <= Y && Y < BOARD_SIZE; };

		for (int d = 0; d < 4; ++d)
		{
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
