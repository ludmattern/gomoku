#pragma once
#include <iostream>
#include <optional>
#include "gomoku/BoardView.hpp"
#include "gomoku/Notation.hpp"

namespace testutil
{

	inline void printBoard(const gomoku::BoardView &b, std::optional<gomoku::Pos> last = {})
	{
		using namespace gomoku;
		auto &n = BOARD_SIZE;
		(void)n;
		std::cout << "    ";
		for (int x = 0; x < BOARD_SIZE; ++x)
			std::cout << ' ' << notation::colLabel(x) << ' ';
		std::cout << "\n";
		for (int y = 0; y < BOARD_SIZE; ++y)
		{
			std::cout << (y + 1 < 10 ? " " : "") << y + 1 << "  ";
			for (int x = 0; x < BOARD_SIZE; ++x)
			{
				bool isLast = last && last->x == x && last->y == y;
				char ch = '.';
				auto c = b.at(x, y);
				if (c == Cell::Black)
					ch = isLast ? 'X' : 'B';
				else if (c == Cell::White)
					ch = isLast ? 'O' : 'W';
				else if (isLast)
					ch = '*';
				std::cout << ' ' << ch << ' ';
			}
			std::cout << "\n";
		}
	}

} // namespace testutil
