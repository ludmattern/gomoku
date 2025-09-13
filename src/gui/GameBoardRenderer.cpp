#include "gui/GameBoardRenderer.hpp"
#include <cmath>
#include <iostream>

namespace gomoku::gui {

static constexpr int N_INTERSECTIONS = 19;
static constexpr int CENTER_INDEX = (N_INTERSECTIONS - 1) / 2;

GameBoardRenderer::GameBoardRenderer() = default;
GameBoardRenderer::~GameBoardRenderer() { cleanup(); }

void GameBoardRenderer::init() { /* nothing for now */ }

void GameBoardRenderer::cleanup()
{
    delete boardSprite_;
    delete pawn1Sprite_;
    delete pawn2Sprite_;
    boardSprite_ = pawn1Sprite_ = pawn2Sprite_ = nullptr;
}

sf::Vector2f GameBoardRenderer::isoToScreen(int i, int j, float tileW, float tileH, float centerX, float centerY)
{
    const float u = static_cast<float>(i - CENTER_INDEX);
    const float v = static_cast<float>(j - CENTER_INDEX);
    return { centerX + (u - v) * (tileW * 0.5f), centerY + (u + v) * (tileH * 0.5f) };
}

void GameBoardRenderer::setTextures(sf::Texture& boardTexture, sf::Texture& pawn1Texture, sf::Texture& pawn2Texture)
{
    delete boardSprite_;
    delete pawn1Sprite_;
    delete pawn2Sprite_;
    boardSprite_ = new sf::Sprite(boardTexture);
    pawn1Sprite_ = new sf::Sprite(pawn1Texture);
    pawn2Sprite_ = new sf::Sprite(pawn2Texture);
    if (!boardSprite_ || !pawn1Sprite_ || !pawn2Sprite_) {
        std::cerr << "Failed to set textures" << std::endl;
    }
}

void GameBoardRenderer::render(sf::RenderWindow& window) const
{
    const auto size = window.getSize();
    const float centerX = static_cast<float>(size.x) * 0.5f;
    const float centerY = static_cast<float>(size.y) * 0.5f;
    const float tileW = std::min(static_cast<float>(size.x) * 0.8f / 18.f,
        static_cast<float>(size.y) * 0.8f * 2.f / 18.f);
    const float tileH = tileW * 0.5f;

    if (boardSprite_) {
        boardSprite_->setPosition({ 6.f, 5.f });
        boardSprite_->setScale({ 1.f, 1.f });
        window.draw(*boardSprite_);
    }
    if (!boardView_)
        return;

    for (int i = 0; i < N_INTERSECTIONS; ++i) {
        for (int j = 0; j < N_INTERSECTIONS; ++j) {
            auto c = boardView_->at(static_cast<uint8_t>(i), static_cast<uint8_t>(j));
            if (c == gomoku::Cell::Empty)
                continue;
            const auto p = isoToScreen(i, j, tileW, tileH, centerX, centerY);
            sf::Sprite* sprite = nullptr;
            if (c == gomoku::Cell::White)
                sprite = pawn1Sprite_;
            else if (c == gomoku::Cell::Black)
                sprite = pawn2Sprite_;
            if (!sprite || !sprite->getTexture())
                continue;
            float pawnSize = tileW * 0.6f;
            float scale = pawnSize / static_cast<float>(sprite->getTexture()->getSize().x);
            sprite->setPosition({ p.x - pawnSize * 0.5f, p.y - pawnSize * 0.5f - 5.f });
            sprite->setScale({ scale, scale });
            window.draw(*sprite);
        }
    }
}

} // namespace gomoku::gui
