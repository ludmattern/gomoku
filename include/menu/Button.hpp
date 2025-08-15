#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button
{
    public:
        enum class State { Normal, Hovered, Pressed };

        Button(void);
        ~Button(void);

        void setBounds(const sf::FloatRect& boundsInView);
        void setText(const std::String& label, const sf::Font& font, unsigned fontSize);
        void setColor(const sf::Color& normal, const sf::Color& hovered, const sf::Color& pressed, const sf::Color& text);
        void setOnClick(std::function<void()> onClick);

        void setState(State state);
        bool contains(const sf::Vector2f& viewPos) const;
        void render(sf::RenderWindow& window);

        void click(void);

    private:
        sf::FloatRect _bounds;
        sf::RectangleShape _rect;
        sf::Text _text;
        std::function<void()> _onClick;
        State _state;
        sf::Color _colorNormal, _colorHovered, _colorPressed, _colorText;
};