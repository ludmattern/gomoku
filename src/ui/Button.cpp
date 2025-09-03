#include "ui/Button.hpp"

#include <algorithm>

Button::Button(void)
    : _shape()
    , _callback()
    , _isHovered(false)
    , _isPressed(false)
    , _scale(1.f)
    , _texture(nullptr)
    , _sprite(nullptr)
{
    _shape.setSize(sf::Vector2f(100, 50));
    _shape.setFillColor(sf::Color(100, 100, 100));
    _shape.setOutlineColor(sf::Color::Black);
    _shape.setOutlineThickness(0.f);
}

Button::~Button(void)
{
}

void Button::setTexture(const sf::Texture* texture)
{
    _texture = texture;
    if (_texture) {
        _sprite = std::make_unique<sf::Sprite>(*_texture);
        auto tex = _texture->getSize();
        _sprite->setScale({ _scale, _scale });
        _shape.setSize({ tex.x * _scale, tex.y * _scale });
        _sprite->setPosition(_shape.getPosition());
    }
}

void Button::setPosition(const sf::Vector2f& position)
{
    _shape.setPosition(position);
    if (_sprite && _texture) {
        _sprite->setPosition(position);
    }
}

void Button::setSize(const sf::Vector2f& size)
{
    _shape.setSize(size);
    if (_sprite && _texture) {
        auto texSize = _texture->getSize();
        float sx = size.x / texSize.x;
        float sy = size.y / texSize.y;
        _scale = std::min(sx, sy);
        _sprite->setScale({ _scale, _scale });
    }
}

void Button::setScale(float scale)
{
    _scale = std::max(0.f, scale);
    if (_sprite && _texture) {
        _sprite->setScale({ _scale, _scale });
        auto tex = _texture->getSize();
        _shape.setSize({ tex.x * _scale, tex.y * _scale });
    } else {
        auto s = _shape.getSize();
        _shape.setSize({ s.x * _scale, s.y * _scale });
    }
}

void Button::setCallback(const std::function<void()>& callback)
{
    _callback = callback;
}

void Button::update(const sf::Time& deltaTime)
{
    (void)deltaTime;
}

void Button::render(sf::RenderTarget& target) const
{
    const auto pos = _shape.getPosition();
    const auto size = _shape.getSize();
    if (size.x <= 0.f || size.y <= 0.f)
        return;

    if (_sprite && _texture) {
        _sprite->setPosition(pos);
        target.draw(*_sprite);
        return;
    }

    sf::RectangleShape rect(size);
    rect.setPosition(pos);
    rect.setFillColor(_shape.getFillColor());
    rect.setOutlineThickness(_shape.getOutlineThickness());
    rect.setOutlineColor(_shape.getOutlineColor());
    target.draw(rect);
}

bool Button::handleInput(const sf::Event& event, const sf::RenderWindow& window)
{
    // basic guard in case size is nonsense
    if (_shape.getSize().x <= 0.f || _shape.getSize().y <= 0.f)
        return false;
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        _isHovered = _shape.getGlobalBounds().contains(mousePos);
        if (_sprite && _texture) {
            _sprite->setColor(_isHovered ? sf::Color(230, 230, 230) : sf::Color(255, 255, 255));
        } else {
            _shape.setFillColor(_isHovered ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (_shape.getGlobalBounds().contains(mousePos)) {
                _isPressed = true;
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (_isPressed) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                if (_shape.getGlobalBounds().contains(mousePos)) {
                    if (_callback) {
                        _callback();
                        _isPressed = false;
                        return true;
                    }
                }
                _isPressed = false;
            }
        }
    }
    return false;
}