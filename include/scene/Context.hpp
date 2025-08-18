#pragma once

#include <SFML/Graphics.hpp>
#include "core/RessourceManager.hpp"

struct Context {
	sf::RenderWindow* window = nullptr;
	RessourceManager* ressourceManager = nullptr;
	bool shouldQuit = false;
	bool inGame = false;
};