#include "GameWindow.hpp"
#include <iostream>
#include "GameBoardRenderer.hpp"

GameWindow::GameWindow(void)
{
    _isRunning = false;
    init();
}

GameWindow::~GameWindow(void)
{
    cleanup();
}

bool GameWindow::isRunning(void)
{
    return _isRunning;
}

void GameWindow::init(void)
{    
    _window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Gomoku");
    _isRunning = true;
}

void GameWindow::handleEvents(void)
{
    while (auto event = _window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            cleanup();
        if (event->is<sf::Event::KeyPressed>())
        {
            auto key = event->getIf<sf::Event::KeyPressed>()->code;
            if (key == sf::Keyboard::Key::Escape)
                cleanup();
        }
    }
}

void GameWindow::cleanup(void)
{
    _window.close();
    _isRunning = false;
}


void GameWindow::render(void)
{
    _window.clear(sf::Color::Blue);
    renderBoard();
    _window.display();
}

void GameWindow::renderBoard(void)
{
    _boardRenderer.render(_window);
}

void GameWindow::run(void)
{
    while (_isRunning)
    {
        handleEvents();
        render();
    }
}