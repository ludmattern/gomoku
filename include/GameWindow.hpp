#ifndef GAME_WINDOW_HPP
# define GAME_WINDOW_HPP

#include "GameBoardRenderer.hpp"
#include "core/RessourceManager.hpp"
#include <SFML/Graphics.hpp>
#include "scene/Context.hpp"
#include "scene/AScene.hpp"
#include <memory>

enum class GameState
{
    MainMenu,
    Game
};

class GameWindow
{
    public:
        GameWindow(void);
        ~GameWindow(void);

        bool isRunning(void);
        void init(void);
        void run(void);
        void handleEvents(void);
        void render(void);
        void cleanup(void);
        void renderBoard(void);

    private:
        sf::RenderWindow _window;
        bool _isRunning;
        Context _context;
        GameBoardRenderer _boardRenderer;
        RessourceManager _ressourceManager;
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