#pragma once

#include "core/RessourceManager.hpp"
#include <SFML/Graphics.hpp>

class GameBoardRenderer;

struct Context {
    sf::RenderWindow* window = nullptr;
    RessourceManager* ressourceManager = nullptr;
    bool shouldQuit = false;
    bool inGame = false;
    bool showGameSelectMenu = false;
    bool showMainMenu = false;
    bool vsAi = false;
    GameBoardRenderer* boardRenderer = nullptr;
};