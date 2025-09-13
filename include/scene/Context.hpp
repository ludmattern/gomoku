#pragma once
#include "gui/ResourceManager.hpp"
#include <SFML/Graphics.hpp>

namespace gomoku::gui {
class GameBoardRenderer;
}

namespace gomoku::scene {

struct Context {
    sf::RenderWindow* window = nullptr;
    gomoku::gui::ResourceManager* resourceManager = nullptr;
    bool shouldQuit = false;
    bool inGame = false;
    bool showGameSelectMenu = false;
    bool showMainMenu = false;
    bool vsAi = false;
    gomoku::gui::GameBoardRenderer* boardRenderer = nullptr;
};

} // namespace gomoku::scene
