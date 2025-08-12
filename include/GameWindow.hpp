#ifndef GAME_WINDOW_HPP
# define GAME_WINDOW_HPP

#include <SFML/Graphics.hpp>
#include "GameBoardRenderer.hpp"

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
    };

#endif