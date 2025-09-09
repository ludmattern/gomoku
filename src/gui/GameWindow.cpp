#include "gui/GameWindow.hpp"
#include "scene/Context.hpp"
#include "scene/GameScene.hpp"
#include "scene/GameSelect.hpp"
#include "scene/MainMenu.hpp"
#include <cmath>
#include <iostream>

GameWindow::GameWindow(void)
    : _resourceManager("default")
    , _backgroundSprite(nullptr)
{
    _isRunning = false;
    _cleaned = false;
    init();
}

GameWindow::~GameWindow(void)
{
}

bool GameWindow::isRunning(void)
{
    return _isRunning;
}

void GameWindow::init(void)
{
    _window.create(
        sf::VideoMode(1920, 1080),
        "Gomoku",
        sf::Style::Close | sf::Style::Titlebar);
    _window.setFramerateLimit(60);
    std::cout << "[INIT] Window created" << std::endl;

    _context = Context();
    _context.window = &_window;
    _context.resourceManager = &_resourceManager;

    if (!_resourceManager.init()) {
        std::cerr << "Failed to initialize ResourceManager" << std::endl;
        return;
    }
    std::cout << "[INIT] ResourceManager ready" << std::endl;

    // Background
    _backgroundSprite = new sf::Sprite(_resourceManager.getTexture("background"));
    // Échelle pour couvrir toute la fenêtre
    _backgroundSprite->setScale(sf::Vector2f(1.0f, 1.0f));
    std::cout << "[INIT] Background sprite created" << std::endl;

    // Shader radial
    _introActive = _radialMask.loadFromFile("assets/shaders/radial_mask.frag", sf::Shader::Type::Fragment);
    _introClock.restart();
    std::cout << "[INIT] Shader loaded? " << (_introActive ? "yes" : "no") << std::endl;

    // Scène: démarrer sur le menu principal
    _currentScene = std::make_unique<MainMenu>(_context);
    _currentScene->onEnter();
    std::cout << "[INIT] MainMenu created" << std::endl;

    _isRunning = true;
}

void GameWindow::handleEvents(void)
{
    sf::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _context.shouldQuit = true;
            return;
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                _context.shouldQuit = true;
                return;
            }
        }

        // Tant que l'intro est active, on ignore les entrées de la scène
        if (_introActive)
            continue;

        if (_currentScene) {
            _currentScene->handleInput(event);
            if (_context.shouldQuit)
                return;
        }
    }
}

void GameWindow::cleanup(void)
{
    if (_cleaned)
        return;
    _cleaned = true;
    _isRunning = false;

    if (_currentScene) {
        _currentScene->onExit();
        _currentScene.reset();
    }

    delete _backgroundSprite;
    _backgroundSprite = nullptr;

    _resourceManager.cleanup();

    if (_window.isOpen())
        _window.close();
}

void GameWindow::render(void)
{
    if (!_window.isOpen())
        return;
    _window.clear(sf::Color::Black);

    if (_backgroundSprite) {
        if (_introActive) {
            const float delay = 1.0f;
            const float duration = 1.2f;
            const auto win = _window.getSize();
            float elapsed = _introClock.getElapsedTime().asSeconds();
            if (elapsed >= delay) {
                float t = std::min((elapsed - delay) / duration, 1.f);
                t = t * t * (3.f - 2.f * t);
                float Rmax = std::hypot(win.x * 0.5f, win.y * 0.5f);
                _radialMask.setUniform("uCenter", sf::Glsl::Vec2(win.x * 0.5f, win.y * 0.5f));
                _radialMask.setUniform("uRadius", t * Rmax);
                _window.draw(*_backgroundSprite, &_radialMask);
                if (t >= 1.f)
                    _introActive = false;
            }
        } else {
            _window.draw(*_backgroundSprite);
        }
    }

    if (!_introActive) {
        if (_currentScene) {
            _currentScene->render(_window);
        }
    }

    _window.display();
}

void GameWindow::run(void)
{
    while (_isRunning) {
        _deltaTime = _clock.restart();
        handleEvents();
        if (!_isRunning)
            break;
        if (_context.inGame) {
            if (!_context.showGameSelectMenu && !_context.showMainMenu) {
                if (_currentScene)
                    _currentScene->onExit();
                std::cout << "[RUN] switch -> GameScene (vsAi=" << (_context.vsAi ? "true" : "false") << ")" << std::endl;
                _currentScene = std::make_unique<GameScene>(_context, _context.vsAi);
                _context.inGame = false;
            }
        }
        if (_context.shouldQuit) {
            cleanup();
            std::exit(0);
        } else if (_context.showGameSelectMenu) {
            if (!_context.inGame && !_context.showMainMenu) {
                if (_currentScene)
                    _currentScene->onExit();
                std::cout << "[RUN] switch -> GameSelect" << std::endl;
                _currentScene = std::make_unique<GameSelectScene>(_context);
                _context.showGameSelectMenu = false;
            }
        } else if (_context.showMainMenu) {
            if (!_context.inGame && !_context.showGameSelectMenu) {
                if (_currentScene)
                    _currentScene->onExit();
                std::cout << "[RUN] switch -> MainMenu" << std::endl;
                _currentScene = std::make_unique<MainMenu>(_context);
                _context.showMainMenu = false;
            }
        }
        if (_currentScene) {
            _currentScene->update(_deltaTime);
        }
        render();
    }
}