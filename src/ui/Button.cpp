#include "ui/Button.hpp"

#include <algorithm>

namespace gomoku::ui {

Button::Button() = default;
Button::~Button() = default;

void Button::setTexture(const sf::Texture* texture)
{
    texture_ = texture;
    if (texture_) {
        sprite_ = std::make_unique<sf::Sprite>(*texture_);
        auto tex = texture_->getSize();
        sprite_->setScale({ scale_, scale_ });
        shape_.setSize({ static_cast<float>(tex.x) * scale_, static_cast<float>(tex.y) * scale_ });
        sprite_->setPosition(shape_.getPosition());
    }
}

void Button::setPosition(const sf::Vector2f& position)
{
    shape_.setPosition(position);
    if (sprite_ && texture_) {
        sprite_->setPosition(position);
    }
}

void Button::setSize(const sf::Vector2f& size)
{
    shape_.setSize(size);
    if (sprite_ && texture_) {
        auto texSize = texture_->getSize();
        float sx = size.x / static_cast<float>(texSize.x);
        float sy = size.y / static_cast<float>(texSize.y);
        scale_ = std::min(sx, sy);
        sprite_->setScale({ scale_, scale_ });
    }
}

void Button::setScale(float scale)
{
    scale_ = std::max(0.f, scale);
    if (sprite_ && texture_) {
        sprite_->setScale({ scale_, scale_ });
        auto tex = texture_->getSize();
        shape_.setSize({ static_cast<float>(tex.x) * scale_, static_cast<float>(tex.y) * scale_ });
    } else {
        auto s = shape_.getSize();
        shape_.setSize({ s.x * scale_, s.y * scale_ });
    }
}

void Button::setCallback(const std::function<void()>& callback)
{
    callback_ = callback;
}

void Button::update(const sf::Time& deltaTime)
{
    (void)deltaTime;
}

void Button::render(sf::RenderTarget& target) const
{
    const auto pos = shape_.getPosition();
    const auto size = shape_.getSize();
    if (size.x <= 0.f || size.y <= 0.f)
        return;

    if (sprite_ && texture_) {
        sprite_->setPosition(pos);
        target.draw(*sprite_);
        return;
    }

    sf::RectangleShape rect(size);
    rect.setPosition(pos);
    rect.setFillColor(shape_.getFillColor());
    rect.setOutlineThickness(shape_.getOutlineThickness());
    rect.setOutlineColor(shape_.getOutlineColor());
    target.draw(rect);
}

bool Button::handleInput(const sf::Event& event, const sf::RenderWindow& window)
{
    if (shape_.getSize().x <= 0.f || shape_.getSize().y <= 0.f)
        return false;
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        isHovered_ = shape_.getGlobalBounds().contains(mousePos);
        if (sprite_ && texture_) {
            sprite_->setColor(isHovered_ ? sf::Color(230, 230, 230) : sf::Color(255, 255, 255));
        } else {
            shape_.setFillColor(isHovered_ ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (shape_.getGlobalBounds().contains(mousePos)) {
                isPressed_ = true;
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isPressed_) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                if (shape_.getGlobalBounds().contains(mousePos)) {
                    if (callback_) {
                        callback_();
                        isPressed_ = false;
                        return true;
                    }
                }
                isPressed_ = false;
            }
        }
    }
    return false;
}

} // namespace gomoku::ui