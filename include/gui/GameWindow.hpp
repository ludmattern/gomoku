#pragma once
#include "gui/GameBoardRenderer.hpp"
#include "gui/ResourceManager.hpp"
#include "scene/AScene.hpp"
#include "scene/Context.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

namespace gomoku::scene {
class AScene;
}

namespace gomoku::gui {

class GameWindow {
public:
    GameWindow();
    ~GameWindow();

    bool isRunning();
    void init();
    void run();
    void handleEvents();
    void render();
    void cleanup();

private:
    sf::RenderWindow window_;
    bool isRunning_ { false };
    gomoku::scene::Context context_;
    ResourceManager resourceManager_ { "default" };
    GameBoardRenderer boardRenderer_;
    sf::Sprite* backgroundSprite_ { nullptr };
    std::unique_ptr<gomoku::scene::AScene> currentScene_;
    sf::Clock clock_;
    sf::Time deltaTime_;
    bool cleaned_ { false };

    sf::Shader radialMask_;
    bool introActive_ = false;
    sf::Clock introClock_;
};

} // namespace gomoku::gui