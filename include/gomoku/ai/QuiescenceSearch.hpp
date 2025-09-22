#pragma once
#include "gomoku/ai/SearchStats.hpp"
#include "gomoku/core/Types.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace gomoku {

class Board;
struct RuleSet;

/**
 * @brief Classe responsable de la recherche de quiescence
 *
 * La recherche de quiescence continue l'évaluation dans les positions "non-calmes"
 * où il y a des menaces tactiques imminentes, évitant l'effet d'horizon.
 */
class QuiescenceSearch {
public:
    struct Config {
        int maxDepth = 4; // Profondeur maximale de quiescence
        bool enabled = true; // Activer/désactiver la quiescence
        int maxTacticalMoves = 8; // Limite du nombre de mouvements tactiques évalués
    };

    struct Result {
        int score;
        std::optional<Move> move;
    };

    QuiescenceSearch();
    explicit QuiescenceSearch(const Config& config);

    // Partage de l'arrêt (timeout/budget) avec le moteur principal
    void setStopCallback(std::function<bool()> cb) { stopCallback_ = std::move(cb); }

    /**
     * @brief Effectue une recherche de quiescence
     * @param board Plateau de jeu
     * @param rules Règles du jeu
     * @param alpha Borne alpha pour l'élagage
     * @param beta Borne beta pour l'élagage
     * @param maxPlayer Joueur maximisant
     * @param evaluateFunction Fonction d'évaluation à utiliser
     * @param stats Statistiques de recherche (optionnel)
     * @param depth Profondeur actuelle de quiescence
     * @return Résultat de la recherche (score et meilleur mouvement)
     */
    Result search(Board& board, const RuleSet& rules, int alpha, int beta,
        Player maxPlayer, std::function<int(const Board&, Player)> evaluateFunction,
        SearchStats* stats = nullptr, int depth = 0);

    /**
     * @brief Vérifie si une position est "calme" (pas de menaces imminentes)
     */
    bool isQuiet(const Board& board, const RuleSet& rules, Player toPlay);

    /**
     * @brief Génère les mouvements tactiquement importants
     */
    std::vector<Move> generateTacticalMoves(Board& board, const RuleSet& rules, Player toPlay);

    /**
     * @brief Détermine si un mouvement est tactiquement important
     */
    bool isTacticalMove(const Board& board, uint8_t x, uint8_t y, Player toPlay) const;

    // Configuration
    void setMaxDepth(int depth) { config_.maxDepth = depth; }
    void setEnabled(bool enabled) { config_.enabled = enabled; }
    void setMaxTacticalMoves(int max) { config_.maxTacticalMoves = max; }

    const Config& getConfig() const { return config_; }

private:
    Config config_;
    mutable unsigned long long visitedNodes_ = 0;
    std::function<bool()> stopCallback_;

    /**
     * @brief Vérifie si la recherche doit s'arrêter (timeout, etc.)
     */
    bool shouldStop(SearchStats* stats) const;
};

} // namespace gomoku