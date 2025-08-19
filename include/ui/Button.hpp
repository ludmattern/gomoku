#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <memory>

class Button
{
    public:
        Button(void);
        ~Button(void);

        void setPosition(const sf::Vector2f& position);
        void setSize(const sf::Vector2f& size);
        void setScale(float scale);      // scale uniforme
        void setCallback(const std::function<void()>& callback);
        void setTexture(const sf::Texture* texture);

        void update(const sf::Time& deltaTime);
        void render(sf::RenderTarget& target) const;
        bool handleInput(const sf::Event& event, const sf::RenderWindow& window);

    private:
        sf::RectangleShape _shape;
        std::function<void()> _callback;
        bool _isHovered;
        bool _isPressed;
        float _scale = 1.f;
        const sf::Texture* _texture; // non possédée
        mutable std::unique_ptr<sf::Sprite> _sprite;  // créé à la volée si besoin
};