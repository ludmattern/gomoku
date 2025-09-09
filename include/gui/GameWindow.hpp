#ifndef GAME_WINDOW_HPP
#define GAME_WINDOW_HPP

#include "GameBoardRenderer.hpp"
#include "gui/ResourceManager.hpp"
#include "scene/AScene.hpp"
#include "scene/Context.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class GameBoardRenderer;

class GameWindow {
public:
    GameWindow(void);
    ~GameWindow(void);

    bool isRunning(void);
    void init(void);
    void run(void);
    void handleEvents(void);
    void render(void);
    void cleanup(void);

private:
    sf::RenderWindow _window;
    bool _isRunning;
    Context _context;
    ResourceManager _resourceManager;
    GameBoardRenderer _boardRenderer;
    sf::Sprite* _backgroundSprite;
    std::unique_ptr<AScene> _currentScene;
    sf::Clock _clock;
    sf::Time _deltaTime;
    bool _cleaned;

    // Animation d’intro (shader radial)
    sf::Shader _radialMask;
    bool _introActive = false;
    sf::Clock _introClock;
};

#endif