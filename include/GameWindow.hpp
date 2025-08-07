#ifndef GAME_WINDOW_HPP
# define GAME_WINDOW_HPP

#include <SFML/Graphics.hpp>

class GameWindow
{
    public:
        GameWindow(void);
        ~GameWindow(void);

    private:
        sf::RenderWindow _window;
};

#endif