// #include <iostream>
// #include <iomanip>
// #include <optional>
// #include <string>
// #include <vector>
// #include <cctype>
// #include <cstdlib>

// #include "gomoku/GameSession.hpp"
// #include "gomoku/Notation.hpp"

// #include <termios.h>
// #include <unistd.h>

// using namespace gomoku;
// namespace note = gomoku::notation;

// // ====== Helpers affichage ======
// static inline void clearScreen() { std::cout << "\033[H\033[J"; }

// static void printBoardLine(const char *L, const char *H, const char *J, const char *R, int n)
// {
// 	std::cout << "   " << L;
// 	for (int i = 0; i < n - 1; ++i)
// 		std::cout << H << J;
// 	std::cout << H << R << "\n";
// }

// static void drawBoard(const BoardView &b, std::optional<Pos> last, std::optional<Pos> cursor)
// {
// 	clearScreen();
// 	const int N = BOARD_SIZE;
// 	const char *V = "│", *H = "───", *TL = "┌", *TR = "┐", *BL = "└", *BR = "┘", *TJ = "┬", *MJ = "┼", *LJ = "├", *RJ = "┤", *BJ = "┴";

// 	std::cout << "\n    ";
// 	for (int x = 0; x < N; ++x)
// 		std::cout << ' ' << note::colLabel(x) << (x + 1 < N ? "  " : "  ");
// 	std::cout << "\n";
// 	printBoardLine(TL, H, TJ, TR, N);

// 	for (int y = 0; y < N; ++y)
// 	{
// 		std::cout << std::setw(2) << (y + 1) << " " << V;
// 		for (int x = 0; x < N; ++x)
// 		{
// 			auto c = b.at(x, y);
// 			bool isLast = last && last->x == x && last->y == y;
// 			bool isCur = cursor && cursor->x == x && cursor->y == y;

// 			const char *g = "·";
// 			if (c == Cell::Black)
// 				g = isLast ? "◉" : "●";
// 			else if (c == Cell::White)
// 				g = isLast ? "◎" : "○";
// 			else if (isLast)
// 				g = "*";
// 			if (isCur)
// 			{
// 				if (c == Cell::Empty)
// 					g = "◇";
// 				else if (c == Cell::Black)
// 					g = "◍";
// 				else
// 					g = "◌";
// 			}

// 			std::cout << ' ' << g << ' ';
// 			if (x + 1 < N)
// 				std::cout << V;
// 		}
// 		std::cout << V << "\n";
// 		if (y + 1 < N)
// 			printBoardLine(LJ, H, MJ, RJ, N);
// 	}
// 	printBoardLine(BL, H, BJ, BR, N);

// 	auto [bp, wp] = b.capturedPairs();
// 	std::cout << "   Captures  ●: " << bp << "  ○: " << wp
// 			  << "   |  To play: " << (b.toPlay() == Player::Black ? "● Black" : "○ White");
// 	if (last)
// 		std::cout << "   |  Last: " << note::colLabel(last->x) << (last->y + 1);
// 	if (cursor)
// 		std::cout << "   |  Cursor: " << note::colLabel(cursor->x) << (cursor->y + 1);
// 	std::cout << "\n"
// 			  << std::flush;
// }

// // ====== Aide & parsing ======
// static void printInGameHelp()
// {
// 	std::cout << "\nCommandes: A1..S19 | h(hint) | u/U(undo) | n(menu) | q(quit)\n"
// 				 "Clavier: flèches = curseur, ENTREE/ESPACE = jouer\n\n";
// }

// // ====== Raw input (flèches) ======
// struct RawTerm
// {
// 	termios old{};
// 	bool active = false;
// 	RawTerm()
// 	{
// 		if (!isatty(STDIN_FILENO))
// 			return;
// 		termios t{};
// 		if (tcgetattr(STDIN_FILENO, &old) == 0)
// 		{
// 			t = old;
// 			t.c_lflag &= ~(ICANON | ECHO);
// 			t.c_cc[VMIN] = 1;
// 			t.c_cc[VTIME] = 0;
// 			if (tcsetattr(STDIN_FILENO, TCSANOW, &t) == 0)
// 				active = true;
// 		}
// 	}
// 	~RawTerm()
// 	{
// 		if (active)
// 			tcsetattr(STDIN_FILENO, TCSANOW, &old);
// 	}
// };
// static int readKey()
// {
// 	unsigned char c;
// 	if (read(STDIN_FILENO, &c, 1) != 1)
// 		return -1;
// 	if (c != 0x1B)
// 		return c;
// 	unsigned char s[2];
// 	if (read(STDIN_FILENO, &s[0], 1) != 1)
// 		return 27;
// 	if (read(STDIN_FILENO, &s[1], 1) != 1)
// 		return 27;
// 	if (s[0] == '[')
// 	{
// 		if (s[1] == 'A')
// 			return 1000;
// 		if (s[1] == 'B')
// 			return 1001;
// 		if (s[1] == 'C')
// 			return 1002;
// 		if (s[1] == 'D')
// 			return 1003;
// 	}
// 	return 27;
// }

// // ====== Menu ======
// enum class Mode
// {
// 	HvH,
// 	HvAI
// };
// struct MenuChoice
// {
// 	Mode mode;
// 	bool humanIsBlack;
// 	bool ok;
// };

// static MenuChoice showMenu()
// {
// 	clearScreen();
// 	std::cout << "====================\n"
// 				 "      GOMOKU\n"
// 				 "====================\n"
// 				 "1) Humain vs Humain\n"
// 				 "2) Humain vs IA\n"
// 				 "q) Quitter\n"
// 				 "\nVotre choix: "
// 			  << std::flush;

// 	std::string line;
// 	if (!std::getline(std::cin, line))
// 		return {Mode::HvH, true, false};
// 	if (line == "q" || line == "Q")
// 		return {Mode::HvH, true, false};
// 	int c = -1;
// 	try
// 	{
// 		c = std::stoi(line);
// 	}
// 	catch (...)
// 	{
// 		c = -1;
// 	}

// 	if (c == 1)
// 		return {Mode::HvH, true, true};

// 	if (c == 2)
// 	{
// 		std::cout << "L'humain joue (1) Noir ou (2) Blanc ? [1]: " << std::flush;
// 		std::getline(std::cin, line);
// 		int s = line.empty() ? 1 : std::atoi(line.c_str());
// 		bool humanBlack = (s != 2);
// 		return {Mode::HvAI, humanBlack, true};
// 	}
// 	return {Mode::HvH, true, false};
// }

// // ====== Partie ======
// static void runGame(Mode mode, bool humanBlack)
// {
// 	// Règles par défaut (mets tes options si besoin)
// 	RuleSet rules{};
// 	GameSession session{rules, Controller::Human, Controller::Human};

// 	// Configure les contrôleurs selon le mode
// 	if (mode == Mode::HvH)
// 	{
// 		session.setController(Player::Black, Controller::Human);
// 		session.setController(Player::White, Controller::Human);
// 	}
// 	else
// 	{
// 		session.setController(Player::Black, humanBlack ? Controller::Human : Controller::AI);
// 		session.setController(Player::White, humanBlack ? Controller::AI : Controller::Human);
// 	}

// 	std::optional<Pos> last{};
// 	Pos cursor{(uint8_t)(BOARD_SIZE / 2), (uint8_t)(BOARD_SIZE / 2)};

// 	// Affichage initial
// 	auto snap = session.snapshot();
// 	drawBoard(*snap.view, snap.lastMove, cursor);
// 	printInGameHelp();

// 	// Si l'IA commence
// 	auto ctrl = [&](Player p)
// 	{ return session.controller(p); };
// 	while (session.snapshot().status == GameStatus::Ongoing && ctrl(session.snapshot().toPlay) == Controller::AI)
// 	{
// 		auto r = session.playAI(450);
// 		if (!r.ok)
// 			break;
// 		snap = session.snapshot();
// 		drawBoard(*snap.view, snap.lastMove, cursor);
// 		if (r.stats)
// 		{
// 			std::cout << "AI: " << note::toString(r.mv->pos)
// 					  << "  depth=" << r.stats->depthReached
// 					  << " nodes=" << r.stats->nodes
// 					  << " time=" << r.stats->timeMs << "ms\n";
// 		}
// 	}

// 	RawTerm raw;
// 	std::string buf;

// 	auto showPrompt = [&]()
// 	{
// 		auto s = session.snapshot();
// 		std::cout << ((s.toPlay == Player::Black) ? "● Black" : "○ White")
// 				  << " (" << (ctrl(Player::Black) == Controller::AI ? "B:AI" : "B:Human")
// 				  << "," << (ctrl(Player::White) == Controller::AI ? "W:AI" : "W:Human") << ") > "
// 				  << buf << std::flush;
// 	};

// 	for (;;)
// 	{
// 		auto s = session.snapshot();
// 		if (s.status != GameStatus::Ongoing)
// 		{
// 			std::cout << "Game finished. (n=menu, q=quit)\n";
// 		}
// 		showPrompt();

// 		int k = readKey();
// 		if (k == -1)
// 			break;

// 		if (k == 'q')
// 		{
// 			std::cout << "\n";
// 			std::exit(0);
// 		}
// 		if (k == 'n')
// 		{
// 			std::cout << "\nRetour au menu...\n";
// 			return;
// 		}

// 		// flèches
// 		if (k == 1000)
// 		{
// 			if (cursor.y > 0)
// 				--cursor.y;
// 			drawBoard(*s.view, s.lastMove, cursor);
// 			continue;
// 		}
// 		if (k == 1001)
// 		{
// 			if (cursor.y + 1 < BOARD_SIZE)
// 				++cursor.y;
// 			drawBoard(*s.view, s.lastMove, cursor);
// 			continue;
// 		}
// 		if (k == 1002)
// 		{
// 			if (cursor.x + 1 < BOARD_SIZE)
// 				++cursor.x;
// 			drawBoard(*s.view, s.lastMove, cursor);
// 			continue;
// 		}
// 		if (k == 1003)
// 		{
// 			if (cursor.x > 0)
// 				--cursor.x;
// 			drawBoard(*s.view, s.lastMove, cursor);
// 			continue;
// 		}

// 		// undo
// 		if (k == 'u')
// 		{
// 			if (!session.undo(1))
// 				std::cout << "Nothing to undo.\n";
// 			drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 			continue;
// 		}
// 		if (k == 'U')
// 		{
// 			if (!session.undo(2))
// 				std::cout << "Nothing to undo.\n";
// 			drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 			continue;
// 		}

// 		// hint
// 		if (k == 'h')
// 		{
// 			SearchStats st{};
// 			auto mv = session.hint(450, &st);
// 			if (!mv)
// 				std::cout << "\nNo suggestion.\n";
// 			else
// 			{
// 				std::cout << "\nHint: " << note::toString(mv->pos)
// 						  << " depth=" << st.depthReached
// 						  << " nodes=" << st.nodes
// 						  << " time=" << st.timeMs << "ms\n";
// 				cursor = mv->pos;
// 			}
// 			drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 			continue;
// 		}

// 		// entrée/espace : jouer selon contrôleur
// 		if (k == '\r' || k == '\n' || k == ' ')
// 		{
// 			auto cur = session.snapshot();
// 			if (ctrl(cur.toPlay) == Controller::AI)
// 			{
// 				auto r = session.playAI(450);
// 				if (r.ok && r.stats)
// 				{
// 					std::cout << "AI: " << note::toString(r.mv->pos)
// 							  << "  depth=" << r.stats->depthReached
// 							  << " nodes=" << r.stats->nodes
// 							  << " time=" << r.stats->timeMs << "ms\n";
// 				}
// 				drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 			}
// 			else
// 			{
// 				auto r = session.playHuman(cursor);
// 				if (!r.ok)
// 				{
// 					std::cout << "Illegal: " << note::toString(cursor) << " — " << r.why << "\n";
// 				}
// 				drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 				// Laisser jouer l'IA si c'est son tour (une ou plusieurs fois s'il y a des fins de coup immédiates)
// 				while (session.snapshot().status == GameStatus::Ongoing &&
// 					   ctrl(session.snapshot().toPlay) == Controller::AI)
// 				{
// 					auto r2 = session.playAI(450);
// 					if (r2.ok && r2.stats)
// 					{
// 						std::cout << "AI: " << note::toString(r2.mv->pos)
// 								  << "  depth=" << r2.stats->depthReached
// 								  << " nodes=" << r2.stats->nodes
// 								  << " time=" << r2.stats->timeMs << "ms\n";
// 					}
// 					drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 					if (!r2.ok)
// 						break;
// 				}
// 			}
// 			continue;
// 		}

// 		// saisie A1..S19
// 		if (k == 127 || k == 8)
// 		{
// 			if (!buf.empty())
// 				buf.pop_back();
// 			continue;
// 		}
// 		if (k >= 32 && k < 127)
// 		{
// 			char ch = (char)k;
// 			if (std::isalpha((unsigned char)ch))
// 			{
// 				ch = (char)std::toupper(ch);
// 				if (ch < 'A' || ch > 'A' + BOARD_SIZE - 1)
// 					continue;
// 			}
// 			if (std::isdigit((unsigned char)ch) || std::isalpha((unsigned char)ch))
// 			{
// 				if (buf.size() < 3)
// 					buf.push_back(ch);
// 			}
// 			if (buf.size() >= 2)
// 			{
// 				if (auto p = note::parse(buf))
// 				{
// 					buf.clear();
// 					if (ctrl(session.snapshot().toPlay) == Controller::Human)
// 					{
// 						auto r = session.playHuman(*p);
// 						if (!r.ok)
// 							std::cout << "Illegal: " << note::toString(*p) << " — " << r.why << "\n";
// 						else
// 							cursor = *p;
// 						drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 						while (session.snapshot().status == GameStatus::Ongoing &&
// 							   ctrl(session.snapshot().toPlay) == Controller::AI)
// 						{
// 							auto r2 = session.playAI(450);
// 							if (r2.ok && r2.stats)
// 							{
// 								std::cout << "AI: " << note::toString(r2.mv->pos)
// 										  << "  depth=" << r2.stats->depthReached
// 										  << " nodes=" << r2.stats->nodes
// 										  << " time=" << r2.stats->timeMs << "ms\n";
// 							}
// 							drawBoard(*session.snapshot().view, session.snapshot().lastMove, cursor);
// 							if (!r2.ok)
// 								break;
// 						}
// 					}
// 					else
// 					{
// 						std::cout << "\nC'est à l'IA (ENTREE pour la faire jouer ou 'n' pour menu).\n";
// 					}
// 				}
// 			}
// 			continue;
// 		}
// 	}
// }

// // ====== main ======
// int main()
// {
// 	for (;;)
// 	{
// 		auto choice = showMenu();
// 		if (!choice.ok)
// 		{
// 			std::cout << "Au revoir.\n";
// 			return 0;
// 		}
// 		runGame(choice.mode, choice.humanIsBlack);
// 	}
// }
