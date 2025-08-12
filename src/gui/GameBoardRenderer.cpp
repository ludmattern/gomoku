#include "GameBoardRenderer.hpp"

GameBoardRenderer::GameBoardRenderer(void)
{
    init();
}

GameBoardRenderer::~GameBoardRenderer(void)
{
    cleanup();
}

void GameBoardRenderer::init(void)
{
    for (int i = 0; i < 19; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            _board[i][j] = CellState::Empty;
        }
    }
}

void GameBoardRenderer::cleanup(void)
{
}

void GameBoardRenderer::updateCell(int x, int y, CellState state)
{
    _board[x][y] = state;
}

void GameBoardRenderer::render(sf::RenderWindow& window)
{
    const int gridSize = 600;
    const int cellSize = gridSize / 18;
    const int startX = (1920 - gridSize) / 3;
    const int startY = (1080 - gridSize) / 4;

    for (int i = 0; i < 19; i++)
    {
        sf::RectangleShape line(sf::Vector2f(gridSize, 2));
        line.setPosition(sf::Vector2f(startX, startY + i * cellSize));  // Correction ici
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    for (int j = 0; j < 19; j++)
    {
        sf::RectangleShape line(sf::Vector2f(2, gridSize));
        line.setPosition(sf::Vector2f(startX + j * cellSize, startY));  // Correction ici
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }
}