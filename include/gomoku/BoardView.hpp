#pragma once
#include <cstdint>
#include <utility>
#include "gomoku/Types.hpp"

namespace gomoku
{

	class BoardView
	{
	public:
		virtual ~BoardView() = default;
		virtual Cell at(uint8_t x, uint8_t y) const = 0;
		virtual Player toPlay() const = 0;
		virtual std::pair<int, int> capturedPairs() const = 0; // {black, white}
		virtual GameStatus status() const = 0;
	};

} // namespace gomoku
