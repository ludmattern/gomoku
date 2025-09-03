#pragma once

#include "Context.hpp"

class AScene {
public:
    explicit AScene(Context& context);
    virtual ~AScene(void) = default;

    virtual void onEnter(void) { };
    virtual void onExit(void) { };

    virtual bool handleInput(sf::Event& event) = 0;
    virtual void update(sf::Time& deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

protected:
    Context& _context;
};