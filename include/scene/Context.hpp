#pragma once

#include <SFML/Graphics.hpp>
#include "core/RessourceManager.hpp"

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