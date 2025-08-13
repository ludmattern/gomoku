#include "GameWindow.hpp"
#include <iostream>
#include <cmath>
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
    _window = sf::RenderWindow(
        sf::VideoMode({1920, 1080}),
        "Gomoku"
    );
    _window.setFramerateLimit(60);
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
        if (event->is<sf::Event::MouseButtonPressed>())
        {
            auto mouseClick = event->getIf<sf::Event::MouseButtonPressed>()->button;
            if (mouseClick == sf::Mouse::Button::Left || mouseClick == sf::Mouse::Button::Right)
            {
                const auto size = _window.getSize();
                const float centerX = static_cast<float>(size.x) * 0.5f;
                const float centerY = static_cast<float>(size.y) * 0.5f;

                const int N = 19;
                const int C = (N - 1) / 2;

                const float tileW = std::min(size.x * 0.8f / 18.f, size.y * 0.8f * 2.f / 18.f);
                const float tileH = tileW * 0.5f;

                sf::Vector2f mp = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));
                const float X = mp.x - centerX;
                const float Y = mp.y - centerY;

                const float u = (Y / (tileH * 0.5f) + X / (tileW * 0.5f)) * 0.5f;
                const float v = (Y / (tileH * 0.5f) - X / (tileW * 0.5f)) * 0.5f;

                int i = static_cast<int>(std::lround(u)) + C;
                int j = static_cast<int>(std::lround(v)) + C;

                i = std::max(0, std::min(18, i));
                j = std::max(0, std::min(18, j));

                // Option sécurité: vérifier proximité réelle de l'intersection visée
                // Recalcule la position écran de l'intersection choisie et valide la distance
                const float ui = static_cast<float>(i - C);
                const float vj = static_cast<float>(j - C);
                const float snappedX = centerX + (ui - vj) * (tileW * 0.5f);
                const float snappedY = centerY + (ui + vj) * (tileH * 0.5f);
                const float dx = snappedX - mp.x;
                const float dy = snappedY - mp.y;
                const float maxDist = std::min(tileW, tileH) * 0.35f;

                if ((dx * dx + dy * dy) <= (maxDist * maxDist))
                {
                    if (mouseClick == sf::Mouse::Button::Left)
                        _boardRenderer.updateCell(i, j, CellState::Player1);
                    else if (mouseClick == sf::Mouse::Button::Right)
                        _boardRenderer.updateCell(i, j, CellState::Player2);
                }
            }
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