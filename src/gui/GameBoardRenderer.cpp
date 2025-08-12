#include "GameBoardRenderer.hpp"
#include <iostream>

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
    const int cellSize = gridSize / 19;
    const int startX = (1920 - gridSize) / 3;
    const int startY = (1080 - gridSize) / 4;

    // Changer de 19 à 20 lignes
    // for (int i = 0; i < 20; i++)  // 20 lignes horizontales
    // {
    //     sf::RectangleShape line(sf::Vector2f(gridSize - 10, 2));
    //     line.setPosition(sf::Vector2f(startX, startY + i * cellSize));
    //     line.setFillColor(sf::Color::White);
    //     window.draw(line);
    // }

    // for (int j = 0; j < 20; j++)  // 20 lignes verticales
    // {
    //     sf::RectangleShape line(sf::Vector2f(2, gridSize - 10));
    //     line.setPosition(sf::Vector2f(startX + j * cellSize, startY));
    //     line.setFillColor(sf::Color::White);
    //     window.draw(line);
    // }
    
    // Dessiner la grille noire (intersections au centre des cases blanches)
    const int blackGridStartX = startX + cellSize / 2;  // Décalé de la moitié d'une case
    const int blackGridStartY = startY + cellSize / 2;  // Décalé de la moitié d'une case

    for (int i = 0; i < 19; i++)
    {
        sf::RectangleShape line(sf::Vector2f(gridSize - (cellSize + 10), 2));  // Plus fine
        line.setPosition(sf::Vector2f(blackGridStartX, blackGridStartY + i * cellSize));
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    for (int j = 0; j < 19; j++)
    {
        sf::RectangleShape line(sf::Vector2f(2, gridSize - (cellSize + 10)));  // Plus fine
        line.setPosition(sf::Vector2f(blackGridStartX + j * cellSize, blackGridStartY));
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    for (int i = 0; i < 19; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            switch (_board[i][j])
            {
                case CellState::Player1:
                {
                    sf::CircleShape circle(cellSize / 3);
                    circle.setPosition(sf::Vector2f(startX + i * cellSize + cellSize / 6, startY + j * cellSize + cellSize / 6));
                    circle.setFillColor(sf::Color::White);
                    window.draw(circle);
                    break;
                }
                case CellState::Player2:
                {

                    sf::CircleShape circle(cellSize / 3);
                    circle.setPosition(sf::Vector2f(startX + i * cellSize + cellSize / 6, startY + j * cellSize + cellSize / 6 ));
                    circle.setFillColor(sf::Color::Black);
                    window.draw(circle);
                    break;
                }
                default:
                    break;
            }
        }
    }

}