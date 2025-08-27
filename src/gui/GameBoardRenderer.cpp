#include "GameBoardRenderer.hpp"
#include <iostream>
#include <cmath>

static constexpr int N_INTERSECTIONS = 19;
static constexpr int CENTER_INDEX = (N_INTERSECTIONS - 1) / 2;

GameBoardRenderer::GameBoardRenderer(void): _boardSprite(nullptr), _pawn1Sprite(nullptr), _pawn2Sprite(nullptr)
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
    delete _boardSprite;
    delete _pawn1Sprite;
    delete _pawn2Sprite;
    _boardSprite = nullptr;
    _pawn1Sprite = nullptr;
    _pawn2Sprite = nullptr;
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

void GameBoardRenderer::setTextures(sf::Texture& boardTexture, sf::Texture& pawn1Texture, sf::Texture& pawn2Texture)
{
    delete _boardSprite;
    delete _pawn1Sprite;
    delete _pawn2Sprite;

    _boardSprite = new sf::Sprite(boardTexture);
    _pawn1Sprite = new sf::Sprite(pawn1Texture);
    _pawn2Sprite = new sf::Sprite(pawn2Texture);
    

    if (!_boardSprite || !_pawn1Sprite || !_pawn2Sprite)
    {
        std::cerr << "Failed to set textures" << std::endl;
        return;
    }
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


    if (_boardSprite)
    {
        _boardSprite->setPosition(sf::Vector2f(6, 5));
        _boardSprite->setScale(sf::Vector2f(1.0f, 1.0f)); // Pas de redimensionnement
        window.draw(*_boardSprite);
    }

    // Pierres aux intersections
    for (int i = 0; i < N_INTERSECTIONS; ++i)
    {
        for (int j = 0; j < N_INTERSECTIONS; ++j)
        {
            if (_board[i][j] == CellState::Empty)
                continue;

            const sf::Vector2f p = isoToScreen(i, j, tileW, tileH, centerX, centerY);

            if (_board[i][j] == CellState::Player1)
            {
                float pawnSize = tileW  * 0.6f;
                float scale = pawnSize / _pawn1Sprite->getTexture()->getSize().x;

                _pawn1Sprite->setPosition(sf::Vector2f(p.x - pawnSize * 0.5f, p.y - pawnSize * 0.5f - 5));
                _pawn1Sprite->setScale(sf::Vector2f(scale, scale));
                window.draw(*_pawn1Sprite);
                continue;
                }

            if (_board[i][j] == CellState::Player2)
            {
                float pawnSize = tileW  * 0.6f;
                float scale = pawnSize / _pawn2Sprite->getTexture()->getSize().x;

                _pawn2Sprite->setPosition(sf::Vector2f(p.x - pawnSize * 0.5f, p.y - pawnSize * 0.5f - 5));
                _pawn2Sprite->setScale(sf::Vector2f(scale, scale));
                window.draw(*_pawn2Sprite);
            }
            }
        }
    }

