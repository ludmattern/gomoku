#include "ui/Button.hpp"
#include <stdexcept>
#include <filesystem>
#include <algorithm>

Button::Button(void)
	: _shape()
	, _font()
	, _text(_font, "", 24)
	, _callback()
	, _isHovered(false)
	, _isPressed(false)
	, _fontLoaded(false)
{
	// Essayer plusieurs chemins possibles
	const char* candidates[] = {
		"assets/fonts/arial.ttf",
		"assets/arial.ttf",
		"fonts/arial.ttf",
		"/System/Library/Fonts/Supplemental/Arial.ttf"
	};
	for (const char* p : candidates) {
		if (std::filesystem::exists(p) && _font.openFromFile(p)) {
			_fontLoaded = true;
			break;
		}
	}

	if (_fontLoaded) {
		_text.setFillColor(sf::Color::Black);
	} else {
		_text.setString("");
	}

	_shape.setSize(sf::Vector2f(100, 50));
	_shape.setFillColor(sf::Color::White);
	_shape.setOutlineColor(sf::Color::Black);
	_shape.setOutlineThickness(2);
}

Button::~Button(void)
{
}

void Button::setCornerRadius(float radius)
{
	_cornerRadius = std::max(0.f, radius);
}

void Button::setText(const std::string& text)
{
	if (_fontLoaded) {
		_text.setString(text);
		auto textRect = _text.getLocalBounds();
		_text.setOrigin({textRect.position.x + textRect.size.x / 2.f,
					  textRect.position.y + textRect.size.y / 2.f});
	}
}

void Button::setPosition(const sf::Vector2f& position)
{
	_shape.setPosition(position);
	if (_fontLoaded) {
		_text.setPosition({position.x + _shape.getSize().x / 2.f,
						 position.y + _shape.getSize().y / 2.f});
	}
}

void Button::setSize(const sf::Vector2f& size)
{
	_shape.setSize(size);
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
	// Dessin d’un rectangle à coins arrondis: 2 rectangles + 4 quarts de cercle
	const auto pos = _shape.getPosition();
	const auto size = _shape.getSize();
	const auto fill = _shape.getFillColor();
	const auto outline = _shape.getOutlineColor();
	const float outlineThickness = _shape.getOutlineThickness();

	const float r = std::min({ _cornerRadius, size.x * 0.5f, size.y * 0.5f });
	if (outlineThickness > 0.f)
	{
		// Passe contour (un peu plus grand)
		const float orad = r + outlineThickness;
		const sf::Vector2f osize = { size.x + 2.f * outlineThickness, size.y + 2.f * outlineThickness };
		const sf::Vector2f opos = { pos.x - outlineThickness, pos.y - outlineThickness };

		// Rects contour
		sf::RectangleShape omid({ osize.x - 2.f * orad, osize.y });
		omid.setPosition({ opos.x + orad, opos.y });
		omid.setFillColor(outline);
		target.draw(omid);

		sf::RectangleShape omidV({ osize.x, osize.y - 2.f * orad });
		omidV.setPosition({ opos.x, opos.y + orad });
		omidV.setFillColor(outline);
		target.draw(omidV);

		auto oquarter = [&](sf::Vector2f center) {
			sf::CircleShape c(orad);
			c.setPointCount(32);
			c.setFillColor(outline);
			c.setPosition({ center.x - orad, center.y - orad });
			target.draw(c);
		};
		oquarter({ opos.x + orad,           opos.y + orad });
		oquarter({ opos.x + osize.x - orad, opos.y + orad });
		oquarter({ opos.x + orad,           opos.y + osize.y - orad });
		oquarter({ opos.x + osize.x - orad, opos.y + osize.y - orad });
	}

	// Passe remplissage
	sf::RectangleShape mid({ size.x - 2.f * r, size.y });
	mid.setPosition({ pos.x + r, pos.y });
	mid.setFillColor(fill);
	target.draw(mid);

	sf::RectangleShape midV({ size.x, size.y - 2.f * r });
	midV.setPosition({ pos.x, pos.y + r });
	midV.setFillColor(fill);
	target.draw(midV);

	auto quarter = [&](sf::Vector2f center) {
		sf::CircleShape c(r);
		c.setPointCount(32);
		c.setFillColor(fill);
		c.setPosition({ center.x - r, center.y - r });
		target.draw(c);
	};
	quarter({ pos.x + r,          pos.y + r });
	quarter({ pos.x + size.x - r, pos.y + r });
	quarter({ pos.x + r,          pos.y + size.y - r });
	quarter({ pos.x + size.x - r, pos.y + size.y - r });

	// Texte
	if (_fontLoaded) {
		target.draw(_text);
	}
}

bool Button::handleInput(const sf::Event& event, const sf::RenderWindow& window)
{
	if (event.is<sf::Event::MouseMoved>()) {
		auto posEvt = event.getIf<sf::Event::MouseMoved>();
		sf::Vector2f mousePos = window.mapPixelToCoords({posEvt->position.x, posEvt->position.y});
		_isHovered = _shape.getGlobalBounds().contains(mousePos);
		_shape.setFillColor(_isHovered ? sf::Color(150,150,150) : sf::Color(255,255,255));
	}

	if (event.is<sf::Event::MouseButtonPressed>()) {
		auto pressed = event.getIf<sf::Event::MouseButtonPressed>();
		if (pressed->button == sf::Mouse::Button::Left) {
			sf::Vector2f mousePos = window.mapPixelToCoords({pressed->position.x, pressed->position.y});
			if (_shape.getGlobalBounds().contains(mousePos)) {
				_isPressed = true;
			}
		}
	}

	if (event.is<sf::Event::MouseButtonReleased>()) {
		auto released = event.getIf<sf::Event::MouseButtonReleased>();
		if (released->button == sf::Mouse::Button::Left) {
			if (_isPressed) {
				sf::Vector2f mousePos = window.mapPixelToCoords({released->position.x, released->position.y});
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