#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <utility>

namespace gomoku
{

	inline constexpr int BOARD_SIZE = 19;

	enum class Player : uint8_t
	{
		Black,
		White
	};
	enum class Cell : uint8_t
	{
		Empty,
		Black,
		White
	};

	struct Pos
	{
		uint8_t x{0}, y{0}; // 0..18
		friend constexpr bool operator==(Pos a, Pos b) { return a.x == b.x && a.y == b.y; }
	};

	struct Move
	{
		Pos pos{};
		Player by{Player::Black};
	};

	struct RuleSet
	{
		bool forbidDoubleThree = true;
		bool allowFiveOrMore = true;
		bool capturesEnabled = true;
		uint8_t captureWinPairs = 5; // 5 paires = 10 pierres
	};

	struct EngineConfig
	{
		RuleSet rules{};
		int maxDepthHint = 6;
		int timeBudgetMs = 450;
		std::size_t ttBytes = 64ull << 20;
		unsigned long long nodeCap = 0; // 0 = illimité (contrôle strict par temps)
		uint32_t randomSeed = 0;
	};

	struct SearchStats
	{
		long long nodes = 0;
		long long qnodes = 0;
		int depthReached = 0;
		int timeMs = 0;
		int ttHits = 0;
		std::vector<Move> principalVariation;
	};

	enum class GameStatus
	{
		Ongoing,
		WinByAlign,
		WinByCapture,
		Draw
	};

} // namespace gomoku
