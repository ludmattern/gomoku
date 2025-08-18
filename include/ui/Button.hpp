#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button
{
    public:
        Button(void);
        ~Button(void);

        void setText(const std::string& text);
        void setPosition(const sf::Vector2f& position);
        void setSize(const sf::Vector2f& size);
        void setCallback(const std::function<void()>& callback);
        void setCornerRadius(float radius);
        void update(const sf::Time& deltaTime);
        void render(sf::RenderTarget& target) const;
        bool handleInput(const sf::Event& event, const sf::RenderWindow& window);

    private:
        sf::RectangleShape _shape;
        sf::Font _font;
        sf::Text _text;
        std::function<void()> _callback;
        bool _isHovered;
        bool _isPressed;
        bool _fontLoaded;
        float _cornerRadius;
};