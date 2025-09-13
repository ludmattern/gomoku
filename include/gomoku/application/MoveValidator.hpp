#pragma once
#include "gomoku/core/Types.hpp"
#include "gomoku/interfaces/IBoardView.hpp"
#include <string>

namespace gomoku::application {

/**
 * Validation basique d'un coup côté service avant tentative d'application sur Board.
 *
 * Sépare la logique de pré-validation (tour du joueur, case libre, partie en cours)
 * de la logique interne plus coûteuse (simulation complète avec captures, etc.).
 */
class MoveValidator {
public:
    struct Result {
        bool ok { false };
        std::string reason; // vide si ok
    };

    Result validate(const IBoardView& board, const RuleSet& rules, const Move& move) const;
};

} // namespace gomoku::application
