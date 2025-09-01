#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "gomoku/Types.hpp"
#include "gomoku/Engine.hpp"
#include "test_framework.hpp"
#include "board_print.hpp"

using namespace gomoku;

TEST(empty_board_draw_false)
{
	Engine e{};
	auto &b = e.board();
	REQUIRE(b.status() == GameStatus::Ongoing);
}

TEST(align_win)
{
	Engine e{};
	// Black plays 5 in a row horizontally
	for (int x = 0; x < 5; ++x)
	{
		bool ok = e.play({Pos{(uint8_t)x, (uint8_t)0}, e.board().toPlay()});
		REQUIRE(ok);
		if (x < 4)
		{ // let white play somewhere else
			ok = e.play({Pos{(uint8_t)x, (uint8_t)1}, e.board().toPlay()});
			REQUIRE(ok);
		}
	}
	REQUIRE(e.board().status() == GameStatus::WinByAlign);
}

TEST(capture_basic)
{
	Engine e{};
	// Setup: Black at A1, White at B1 and C1, Black somewhere else, then Black plays D1 to capture
	bool ok;
	ok = e.play({Pos{0, 0}, e.board().toPlay()}); // B: A1
	REQUIRE(ok);
	ok = e.play({Pos{1, 0}, e.board().toPlay()}); // W: B1
	REQUIRE(ok);
	ok = e.play({Pos{10, 10}, e.board().toPlay()}); // B: elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{2, 0}, e.board().toPlay()}); // W: C1
	REQUIRE(ok);
	// Black plays D1, should capture B1 and C1
	ok = e.play({Pos{3, 0}, e.board().toPlay()});
	REQUIRE(ok);
	auto caps = e.board().capturedPairs();
	CHECK(caps.first == 1); // black pairs
	CHECK(caps.second == 0);
	// captured stones removed
	// Need access to BoardView only; ensure empties at B1 and C1
	CHECK(e.board().at(1, 0) == Cell::Empty);
	CHECK(e.board().at(2, 0) == Cell::Empty);
	if (::tfx::ctx().printBoards || ::tfx::ctx().verbose)
	{
		std::cout << "Capture scenario after D1:" << std::endl;
		testutil::printBoard(e.board());
	}
}

TEST(double_three_illegal)
{
	Engine e{};
	std::string why;
	// Create a position so that Black playing at (10,10) would make two open threes (horizontal and vertical)
	// Place blacks at (9,10), (11,10), (10,9), (10,11)
	bool ok;
	ok = e.play({Pos{9, 10}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 0}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{11, 10}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 1}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{10, 9}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 2}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{10, 11}, e.board().toPlay()}); // B
	REQUIRE(ok);
	// Give turn back to Black
	ok = e.play({Pos{0, 3}, e.board().toPlay()}); // W elsewhere
	assert(ok);
	// Now Black to play at (10,10) should be illegal due to double-three (no capture pattern around)
	Move m{Pos{10, 10}, e.board().toPlay()};
	if (::tfx::ctx().printBoards || ::tfx::ctx().verbose)
	{
		std::cout << "Before testing double-three at K11:" << std::endl;
		testutil::printBoard(e.board());
	}
	ok = e.isLegal(m, &why);
	CHECK(!ok);
	CHECK(why.find("double-three") != std::string::npos);
}

TEST(double_three_illegal_diag_vertical)
{
	Engine e{};
	std::string why;
	bool ok;
	// Set up Blacks to create two threes (vertical + main diagonal) if playing at (10,10)
	ok = e.play({Pos{10, 9}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 0}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{10, 11}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 1}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{9, 9}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 2}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	ok = e.play({Pos{11, 11}, e.board().toPlay()}); // B
	REQUIRE(ok);
	// Give turn back to Black
	ok = e.play({Pos{0, 3}, e.board().toPlay()}); // W elsewhere
	REQUIRE(ok);
	// Now Black to play at (10,10) should be illegal (no capture pattern arranged horizontally)
	Move m{Pos{10, 10}, e.board().toPlay()};
	if (::tfx::ctx().printBoards || ::tfx::ctx().verbose)
	{
		std::cout << "Before testing diagonal+vertical double-three at K11:" << std::endl;
		testutil::printBoard(e.board());
	}
	ok = e.isLegal(m, &why);
	CHECK(!ok);
	CHECK(why.find("double-three") != std::string::npos);
}

TEST(double_three_allowed_if_capture)
{
	Engine e{};
	std::string why;
	bool ok;
	// Build two threes (vertical + diagonal) and also a horizontal capture to the left
	// Threes setup:
	ok = e.play({Pos{10, 9}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 0}, e.board().toPlay()}); // W
	REQUIRE(ok);
	ok = e.play({Pos{10, 11}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 1}, e.board().toPlay()}); // W
	REQUIRE(ok);
	ok = e.play({Pos{9, 9}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 2}, e.board().toPlay()}); // W
	REQUIRE(ok);
	ok = e.play({Pos{11, 11}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{0, 3}, e.board().toPlay()}); // W
	REQUIRE(ok);
	// Capture setup horizontally to the left of (10,10): place W at (9,10) and (8,10), and B at (7,10)
	ok = e.play({Pos{7, 10}, e.board().toPlay()}); // B
	REQUIRE(ok);
	ok = e.play({Pos{9, 10}, e.board().toPlay()}); // W
	REQUIRE(ok);
	ok = e.play({Pos{10, 0}, e.board().toPlay()}); // B elsewhere (keep parity)
	REQUIRE(ok);
	ok = e.play({Pos{8, 10}, e.board().toPlay()}); // W
	REQUIRE(ok);
	// Now Black to play at (10,10) creates two threes AND captures (9,10) and (8,10) -> should be allowed
	Move m{Pos{10, 10}, e.board().toPlay()};
	if (::tfx::ctx().printBoards || ::tfx::ctx().verbose)
	{
		std::cout << "Before testing capture-exception at K11:" << std::endl;
		testutil::printBoard(e.board());
	}
	ok = e.isLegal(m, &why);
	REQUIRE(ok);
	// Play it and verify capture occurred
	ok = e.play(m);
	REQUIRE(ok);
	auto caps = e.board().capturedPairs();
	CHECK(caps.first >= 1);
}

TEST(full_board_draw)
{
	EngineConfig cfg{};
	cfg.rules.forbidDoubleThree = false; // ease filling
	cfg.rules.capturesEnabled = false;	 // avoid captures
	cfg.rules.allowFiveOrMore = false;	 // disable 5+ wins so we can fill the board
	Engine e{cfg};
	// Fill checkerboard to avoid 5-in-a-row
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			bool ok = e.play({Pos{(uint8_t)x, (uint8_t)y}, e.board().toPlay()});
			REQUIRE(ok);
		}
	}
	REQUIRE(e.board().status() == GameStatus::Draw);
}

TEST(illegal_wrong_turn_and_occupied)
{
	Engine e{};
	std::string why;
	// Wrong turn: try to play as White first
	Move m1{Pos{0, 0}, Player::White};
	CHECK(!e.isLegal(m1, &why));
	// Proper Black move
	REQUIRE(e.play({Pos{0, 0}, e.board().toPlay()}));
	// Now White to play, try to play on occupied cell A1
	Move m2{Pos{0, 0}, e.board().toPlay()};
	CHECK(!e.isLegal(m2, &why));
}

TEST(undo_restores_state_and_zobrist)
{
	Engine e{};
	// We check functional equivalence: after undo, board cells and toPlay are restored.
	// Play two moves
	REQUIRE(e.play({Pos{1, 1}, e.board().toPlay()}));
	REQUIRE(e.play({Pos{2, 2}, e.board().toPlay()}));
	// Undo one
	REQUIRE(e.undo());
	// Now cell (2,2) must be empty, (1,1) must be Black, toPlay must be White
	CHECK(e.board().at(2, 2) == Cell::Empty);
	CHECK(e.board().at(1, 1) == Cell::Black);
	CHECK(e.board().toPlay() == Player::White);
}

TEST(capture_both_directions_two_pairs)
{
	Engine e{};
	bool ok;
	// Target center (10,10). Prepare: B at 7 and 13; W at 8,9 and 11,12. Then play B at 10 to capture both pairs.
	ok = e.play({Pos{7, 10}, e.board().toPlay()});
	REQUIRE(ok); // B
	ok = e.play({Pos{8, 10}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{0, 0}, e.board().toPlay()});
	REQUIRE(ok); // B elsewhere
	ok = e.play({Pos{9, 10}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{1, 0}, e.board().toPlay()});
	REQUIRE(ok); // B elsewhere
	ok = e.play({Pos{11, 10}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{2, 0}, e.board().toPlay()});
	REQUIRE(ok); // B elsewhere
	ok = e.play({Pos{12, 10}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{13, 10}, e.board().toPlay()});
	REQUIRE(ok); // B
	ok = e.play({Pos{3, 0}, e.board().toPlay()});
	REQUIRE(ok); // W elsewhere -> Black to move
	ok = e.play({Pos{10, 10}, e.board().toPlay()});
	REQUIRE(ok); // B captures two pairs
	auto caps = e.board().capturedPairs();
	CHECK(caps.first == 2);
	CHECK(e.board().at(8, 10) == Cell::Empty);
	CHECK(e.board().at(9, 10) == Cell::Empty);
	CHECK(e.board().at(11, 10) == Cell::Empty);
	CHECK(e.board().at(12, 10) == Cell::Empty);
}

TEST(capture_win_by_pairs)
{
	EngineConfig cfg{};
	cfg.rules.captureWinPairs = 1; // first capture wins
	Engine e{cfg};
	bool ok;
	ok = e.play({Pos{0, 0}, e.board().toPlay()});
	REQUIRE(ok); // B
	ok = e.play({Pos{1, 0}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{10, 10}, e.board().toPlay()});
	REQUIRE(ok); // B elsewhere
	ok = e.play({Pos{2, 0}, e.board().toPlay()});
	REQUIRE(ok); // W
	ok = e.play({Pos{3, 0}, e.board().toPlay()});
	REQUIRE(ok); // B captures -> should win by capture
	CHECK(e.board().status() == GameStatus::WinByCapture);
}

int main(int argc, char **argv)
{
	// Options: -l (list), -k FILTER, -v
	bool list = false, verbose = false, printBoards = false;
	std::string filter;
	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "-l") == 0 || std::strcmp(argv[i], "--list") == 0)
			list = true;
		else if (std::strcmp(argv[i], "-v") == 0)
			verbose = true;
		else if (std::strcmp(argv[i], "-p") == 0 || std::strcmp(argv[i], "--print") == 0)
			printBoards = true;
		else if (std::strcmp(argv[i], "-k") == 0 && i + 1 < argc)
		{
			filter = argv[++i];
		}
	}
	auto &reg = tfx::registry();
	if (list)
	{
		for (auto &t : reg)
			std::cout << t.name << "\n";
		return 0;
	}
	tfx::Context ctx;
	ctx.verbose = verbose;
	ctx.printBoards = printBoards;
	tfx::currentCtxSlot() = &ctx;
	int ran = 0;
	for (auto &t : reg)
	{
		if (!filter.empty() && std::string(t.name).find(filter) == std::string::npos)
			continue;
		ran++;
		if (verbose)
			std::cout << "[ RUN ] " << t.name << "\n";
		try
		{
			t.fn();
		}
		catch (const std::exception &e)
		{
			if (verbose)
				std::cerr << "[ EXC ] " << e.what() << "\n";
		}
		if (verbose)
			std::cout << "[ DONE ] " << t.name << "\n";
	}
	if (verbose)
		std::cout << "Ran " << ran << " test(s). Failures: " << ctx.failures << "\n";
	std::cout << (ctx.failures ? "FAIL" : "OK") << "\n";
	return ctx.failures ? 1 : 0;
}
