#pragma once

#include "scene/AScene.hpp"
#include <memory>

namespace gomoku::scene {

class SceneManager {
public:
    explicit SceneManager(Context& context)
        : context_(context)
    {
    }
    ~SceneManager() = default;

    void changeScene(std::unique_ptr<AScene> scene);

    void handleInput(sf::Event& event);

    void update(sf::Time& deltaTime);
    void render(sf::RenderTarget& target) const;

private:
    Context& context_;
    std::unique_ptr<AScene> currentScene_;
};

} // namespace gomoku::scene
