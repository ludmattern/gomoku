#include "GameBoardRenderer.hpp"
#include <iostream>
#include <cmath>

static constexpr int N_INTERSECTIONS = 19;
static constexpr int CENTER_INDEX = (N_INTERSECTIONS - 1) / 2;

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
    if (x < 0 || x >= 19 || y < 0 || y >= 19)
        return;
    _board[x][y] = state;
}

sf::Vector2f GameBoardRenderer::isoToScreen(int i, int j, float tileW, float tileH, float centerX, float centerY)
{
    const float u = static_cast<float>(i - CENTER_INDEX);
    const float v = static_cast<float>(j - CENTER_INDEX);
    return sf::Vector2f(
        centerX + (u - v) * (tileW * 0.5f),
        centerY + (u + v) * (tileH * 0.5f)
    );
}

void GameBoardRenderer::render(sf::RenderWindow& window)
{
    // Taille fenêtre et centre
    const auto size = window.getSize();
    const float centerX = static_cast<float>(size.x) * 0.5f;
    const float centerY = static_cast<float>(size.y) * 0.5f;

    // Vraie isométrie: tileH = tileW / 2
    const float tileW = std::min(size.x * 0.8f / 18.f, size.y * 0.8f * 2.f / 18.f);
    const float tileH = tileW * 0.5f;

    // Grille: lignes i = const
    for (int i = 0; i < N_INTERSECTIONS; ++i)
    {
        sf::Vector2f a = isoToScreen(i, 0,  tileW, tileH, centerX, centerY);
        sf::Vector2f b = isoToScreen(i, 18, tileW, tileH, centerX, centerY);
        const float dx = b.x - a.x, dy = b.y - a.y;
        const float len = std::sqrt(dx*dx + dy*dy);
        sf::RectangleShape line(sf::Vector2f(len, 2.f));
        line.setPosition(a);
        line.setRotation(sf::radians(std::atan2(dy, dx)));
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    // Grille: lignes j = const
    for (int j = 0; j < N_INTERSECTIONS; ++j)
    {
        sf::Vector2f a = isoToScreen(0,  j, tileW, tileH, centerX, centerY);
        sf::Vector2f b = isoToScreen(18, j, tileW, tileH, centerX, centerY);
        const float dx = b.x - a.x, dy = b.y - a.y;
        const float len = std::sqrt(dx*dx + dy*dy);
        sf::RectangleShape line(sf::Vector2f(len, 2.f));
        line.setPosition(a);
        line.setRotation(sf::radians(std::atan2(dy, dx)));
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    // Pierres aux intersections
    for (int i = 0; i < N_INTERSECTIONS; ++i)
    {
        for (int j = 0; j < N_INTERSECTIONS; ++j)
        {
            if (_board[i][j] == CellState::Empty)
                continue;

            const sf::Vector2f p = isoToScreen(i, j, tileW, tileH, centerX, centerY);
            const float r = tileW * 0.3f;
            sf::CircleShape stone(r);
            stone.setPosition(sf::Vector2f(p.x - r, p.y - r));
            stone.setFillColor(_board[i][j] == CellState::Player1 ? sf::Color::White : sf::Color::Black);
            window.draw(stone);
        }
    }
}