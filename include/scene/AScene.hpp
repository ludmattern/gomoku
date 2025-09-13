#pragma once

#include "scene/Context.hpp"
#include <SFML/Graphics.hpp>

namespace gomoku::scene {

class AScene {
public:
    explicit AScene(Context& context);
    virtual ~AScene() = default;

    virtual void onEnter() { }
    virtual void onExit() { }

    virtual bool handleInput(sf::Event& event) = 0;
    virtual void update(sf::Time& deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

protected:
    Context& context_;
};

} // namespace gomoku::scene
