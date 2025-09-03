#pragma once

#include "AScene.hpp"
#include <memory>

class SceneManager {
public:
    explicit SceneManager(Context& context)
        : _context(context) { };
    ~SceneManager(void) = default;

    void changeScene(std::unique_ptr<AScene> scene);

    void handleInput(sf::Event& event);

    void update(sf::Time& deltaTime);
    void render(sf::RenderTarget& target) const;

private:
    Context& _context;
    std::unique_ptr<AScene> _currentScene;
};