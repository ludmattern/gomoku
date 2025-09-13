#pragma once

#include "gomoku/interfaces/IBoardView.hpp"
#include <SFML/Graphics.hpp>
#include <optional>

namespace gomoku::gui {

class GameBoardRenderer {
public:
    GameBoardRenderer();
    ~GameBoardRenderer();

    void init();
    void cleanup();

    void render(sf::RenderWindow& window) const;

    void setTextures(sf::Texture& boardTexture, sf::Texture& pawn1Texture, sf::Texture& pawn2Texture);
    void setBoardView(const gomoku::IBoardView* view) { boardView_ = view; }

    static sf::Vector2f isoToScreen(int i, int j, float tileW, float tileH, float centerX, float centerY);

private:
    const gomoku::IBoardView* boardView_ = nullptr; // not owned
    sf::Sprite* boardSprite_ = nullptr;
    sf::Sprite* pawn1Sprite_ = nullptr;
    sf::Sprite* pawn2Sprite_ = nullptr;
};

} // namespace gomoku::gui
