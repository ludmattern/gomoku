#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>

namespace gomoku::ui {

class Button {
public:
    Button();
    ~Button();

    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    void setScale(float scale); // uniform scale
    void setCallback(const std::function<void()>& callback);
    void setHoverCallback(const std::function<void()>& onHover);
    void setTexture(const sf::Texture* texture);

    void update(const sf::Time& deltaTime);
    void render(sf::RenderTarget& target) const;
    bool handleInput(const sf::Event& event, const sf::RenderWindow& window);

private:
    sf::RectangleShape shape_;
    std::function<void()> callback_;
    std::function<void()> onHover_;
    bool isHovered_ { false };
    bool wasHovered_ { false };
    bool isPressed_ { false };
    float scale_ = 1.f;
    const sf::Texture* texture_ { nullptr }; // not owned
    mutable std::unique_ptr<sf::Sprite> sprite_; // lazily created
};

} // namespace gomoku::ui