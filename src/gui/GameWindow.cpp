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
    const int gridSize = 600;
    const int startX = (1920 - gridSize) / 3;
    const int startY = (1080 - gridSize) / 4;
    const int cellSize = gridSize / 19;
    
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
            if (mouseClick == sf::Mouse::Button::Left)
            {
                auto mousePosition = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));
                if (mousePosition.x > startX && mousePosition.x < startX + gridSize && mousePosition.y > startY && mousePosition.y < startY + gridSize)
                {
                    int gridX = (mousePosition.x - startX) / cellSize;
                    int gridY = (mousePosition.y - startY) / cellSize;
                    std::cout << "Mouse clicked at: " << mousePosition.x << ", " << mousePosition.y << std::endl;
                    if (gridX >= 0 && gridX < 19 && gridY >= 0 && gridY < 19)
                    {
                        std::cout << "Case clicked at: " << static_cast<int>(gridX) << ", " << static_cast<int>(gridY) << std::endl;
                        _boardRenderer.updateCell(gridX, gridY, CellState::Player1);  // Correction ici
                    }
                }
            }
            if (mouseClick == sf::Mouse::Button::Right)
            {
                auto mousePosition = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));
                if (mousePosition.x > startX && mousePosition.x < startX + gridSize && mousePosition.y > startY && mousePosition.y < startY + gridSize)
                {
                    int gridX = (mousePosition.x - startX) / cellSize;
                    int gridY = (mousePosition.y - startY) / cellSize;
                    if (gridX >= 0 && gridX < 19 && gridY >= 0 && gridY < 19)
                    {
                        std::cout << "Case clicked at: " << static_cast<int>(gridX) << ", " << static_cast<int>(gridY) << std::endl;
                        _boardRenderer.updateCell(gridX, gridY, CellState::Player2);  // Correction ici
                    }
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