#include "gui/GameWindow.hpp"
#include "scene/GameScene.hpp"
#include "scene/GameSelect.hpp"
#include "scene/Settings.hpp"
#include "scene/MainMenu.hpp"
#include <cmath>
#include <iostream>

namespace gomoku::gui {

using gomoku::scene::Context;
using gomoku::scene::GameScene;
using gomoku::scene::GameSelectScene;
using gomoku::scene::MainMenu;

GameWindow::GameWindow() { init(); }
GameWindow::~GameWindow() = default;

bool GameWindow::isRunning() { return isRunning_; }

void GameWindow::init()
{
    window_.create({ 1920, 1080 }, "Gomoku", sf::Style::Close | sf::Style::Titlebar);
    window_.setFramerateLimit(60);
    std::cout << "[INIT] Window created" << std::endl;

    context_ = Context();
    context_.window = &window_;
    context_.resourceManager = &resourceManager_;
    context_.music = &music_;
    sfxVoices_.resize(8); // 8 voix SFX
    context_.sfxVoices = &sfxVoices_;

    if (!resourceManager_.init()) {
        std::cerr << "Failed to initialize ResourceManager" << std::endl;
        return;
    }
    std::cout << "[INIT] ResourceManager ready" << std::endl;

    backgroundSprite_ = new sf::Sprite(resourceManager_.getTexture("background"));
    backgroundSprite_->setScale({ 1.f, 1.f });
    std::cout << "[INIT] Background sprite created" << std::endl;

    introActive_ = radialMask_.loadFromFile("assets/shaders/radial_mask.frag", sf::Shader::Type::Fragment);
    introClock_.restart();
    std::cout << "[INIT] Shader loaded? " << (introActive_ ? "yes" : "no") << std::endl;

    // Start menu music if available (silent if missing)
    if (music_.openFromFile("assets/audio/default/menu_theme.ogg")) {
        music_.setLoop(true);
        music_.setVolume(10.f);
        music_.play();
    }

    currentScene_ = std::make_unique<MainMenu>(context_);
    currentScene_->onEnter();
    std::cout << "[INIT] MainMenu created" << std::endl;

    isRunning_ = true;
}

void GameWindow::handleEvents()
{
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            context_.shouldQuit = true;
            return;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            context_.shouldQuit = true;
            return;
        }
        if (introActive_)
            continue;
        if (currentScene_) {
            currentScene_->handleInput(event);
            if (context_.shouldQuit)
                return;
        }
    }
}

void GameWindow::cleanup()
{
    if (cleaned_)
        return;
    cleaned_ = true;
    isRunning_ = false;
    if (currentScene_) {
        currentScene_->onExit();
        currentScene_.reset();
    }
    delete backgroundSprite_;
    backgroundSprite_ = nullptr;
    resourceManager_.cleanup();
    if (window_.isOpen())
        window_.close();
}

void GameWindow::render()
{
    if (!window_.isOpen())
        return;
    window_.clear(sf::Color::Black);
    if (backgroundSprite_) {
        if (introActive_) {
            const float delay = 1.f;
            const float duration = 1.2f;
            auto win = window_.getSize();
            float elapsed = introClock_.getElapsedTime().asSeconds();
            if (elapsed >= delay) {
                float t = std::min((elapsed - delay) / duration, 1.f);
                t = t * t * (3.f - 2.f * t);
                float Rmax = std::hypot(static_cast<float>(win.x) * 0.5f, static_cast<float>(win.y) * 0.5f);
                radialMask_.setUniform("uCenter", sf::Glsl::Vec2(static_cast<float>(win.x) * 0.5f, static_cast<float>(win.y) * 0.5f));
                radialMask_.setUniform("uRadius", t * Rmax);
                window_.draw(*backgroundSprite_, &radialMask_);
                if (t >= 1.f)
                    introActive_ = false;
            }
        } else {
            window_.draw(*backgroundSprite_);
        }
    }
    if (!introActive_ && currentScene_)
        currentScene_->render(window_);
    window_.display();
}

void GameWindow::run()
{
    while (isRunning_) {
        deltaTime_ = clock_.restart();
        handleEvents();
        if (!isRunning_)
            break;
        // Propagation changement de thème
        if (context_.themeChanged) {
            // Rebind background
            if (backgroundSprite_) {
                try {
                    backgroundSprite_->setTexture(resourceManager_.getTexture("background"), true);
                } catch (...) {}
            }
            if (currentScene_)
                currentScene_->onThemeChanged();
            context_.themeChanged = false;
        }
        if (context_.inGame && !context_.showGameSelectMenu && !context_.showMainMenu) {
            if (currentScene_)
                currentScene_->onExit();
            std::cout << "[RUN] switch -> GameScene (vsAi=" << (context_.vsAi ? "true" : "false") << ")" << std::endl;
            currentScene_ = std::make_unique<GameScene>(context_, context_.vsAi);
            context_.inGame = false;
        }
        if (context_.shouldQuit) {
            cleanup();
            std::exit(0);
        } else if (context_.showGameSelectMenu && !context_.inGame && !context_.showMainMenu) {
            if (currentScene_)
                currentScene_->onExit();
            std::cout << "[RUN] switch -> GameSelect" << std::endl;
            currentScene_ = std::make_unique<GameSelectScene>(context_);
            context_.showGameSelectMenu = false;
        } else if (context_.showSettingsMenu && !context_.inGame && !context_.showMainMenu && !context_.showGameSelectMenu) {
            if (currentScene_)
                currentScene_->onExit();
            std::cout << "[RUN] switch -> Settings" << std::endl;
            currentScene_ = std::make_unique<gomoku::scene::SettingsScene>(context_);
            context_.showSettingsMenu = false;
        } else if (context_.showMainMenu && !context_.inGame && !context_.showGameSelectMenu) {
            if (currentScene_)
                currentScene_->onExit();
            std::cout << "[RUN] switch -> MainMenu" << std::endl;
            currentScene_ = std::make_unique<MainMenu>(context_);
            context_.showMainMenu = false;
        }
        if (currentScene_)
            currentScene_->update(deltaTime_);
        render();
    }
}

} // namespace gomoku::gui