#pragma once

#include "gui/ResourceManager.hpp"
#include <SFML/Graphics.hpp>

class GameBoardRenderer;

struct Context {
    sf::RenderWindow* window = nullptr;
    ResourceManager* resourceManager = nullptr;
    bool shouldQuit = false;
    bool inGame = false;
    bool showGameSelectMenu = false;
    bool showMainMenu = false;
    bool vsAi = false;
    GameBoardRenderer* boardRenderer = nullptr;
};