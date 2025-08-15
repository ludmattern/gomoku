#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Button.hpp"

class MainMenu
{
    public:
        MainMenu(void);
        ~MainMenu(void);

        void init(float viewWidth, float viewHeight, const sf::Font& font);

        void handleMouseMoved(const sf::RenderWindow& window);
        void handleMousePressed(const sf::RenderWindow& window, sf::Mouse::Button button);
        void handleMouseReleased(const sf::RenderWindow& window, sf::Mouse::Button button);

        void render(sf::RenderWindow& window);

        bool playRequestedAndReset(void);

    private:
        std::vector<Button> _buttons;
        bool _playRequested;
        int _hovered;
        int _pressed;
};