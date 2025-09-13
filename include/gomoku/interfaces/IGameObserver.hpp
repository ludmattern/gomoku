#pragma once
#include "gomoku/core/Types.hpp"
#include <optional>

namespace gomoku {

class IBoardView; // forward

/**
 * @brief Interface d'observation des évènements de partie.
 *
 * Permet à la couche UI / logging / analytics d'être notifiée
 * sans devoir "poll" en permanence l'état du service.
 */
class IGameObserver {
public:
    virtual ~IGameObserver() = default;

    // Partie (re)démarrée
    virtual void onGameStarted(const RuleSet& /*rules*/, const IBoardView& /*board*/) { }

    // Coup joué avec succès
    virtual void onMovePlayed(const Move& /*move*/, const IBoardView& /*board*/, GameStatus /*status*/) { }

    // Annulation effectuée
    virtual void onUndo(const IBoardView& /*board*/, GameStatus /*status*/) { }

    // Fin de partie (victoire / nulle / autre)
    virtual void onGameEnded(GameStatus /*finalStatus*/, const IBoardView& /*board*/) { }
};

} // namespace gomoku
