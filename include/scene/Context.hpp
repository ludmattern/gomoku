#pragma once
#include "gui/ResourceManager.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace gomoku::gui {
class GameBoardRenderer;
}

namespace gomoku::scene {

struct Context {
    sf::RenderWindow* window = nullptr;
    gomoku::gui::ResourceManager* resourceManager = nullptr;
    sf::Music* music = nullptr;
    std::vector<sf::Sound>* sfxVoices = nullptr; // pool de voix SFX
    bool shouldQuit = false;
    bool inGame = false;
    bool showGameSelectMenu = false;
    bool showSettingsMenu = false;
    bool showMainMenu = false;
    bool vsAi = false;
    gomoku::gui::GameBoardRenderer* boardRenderer = nullptr;
    std::string theme = "default";
    bool themeChanged = false;
    // Audio settings
    bool sfxEnabled = true;
    bool musicEnabled = true;
    float sfxVolume = 100.f;   // master SFX volume (0-100)
    float musicVolume = 10.f;  // default music volume (0-100)
};

} // namespace gomoku::scene
