#pragma once
#include <optional>
#include <string>
#include "gomoku/Types.hpp"

namespace gomoku::notation
{

	// "A1" -> Pos ; accepte A1..S19
	std::optional<Pos> parse(const std::string &in);

	// Pos -> "A1"
	std::string toString(Pos p);

	// "A".."S" pour afficher les colonnes
	inline std::string colLabel(int x) { return std::string(1, char('A' + x)); }

} // namespace gomoku::notation
