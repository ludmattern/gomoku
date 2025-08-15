#ifndef GAME_WINDOW_HPP
# define GAME_WINDOW_HPP

#include "GameBoardRenderer.hpp"
#include "core/RessourceManager.hpp"
#include <SFML/Graphics.hpp>

enum class GameState
{
    MainMenu,
    Game
};

class GameWindow
{
    public:
        GameWindow(void);
        ~GameWindow(void);

        bool isRunning(void);
        void init(void);
        void run(void);
        void handleEvents(void);
        void render(void);
        void cleanup(void);
        void renderBoard(void);

    private:
        sf::RenderWindow _window;
        bool _isRunning;
        GameBoardRenderer _boardRenderer;
        RessourceManager _ressourceManager;
        sf::Sprite* _backgroundSprite;
    };

#endif