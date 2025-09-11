#ifndef GAME_BOARD_RENDERER_HPP
#define GAME_BOARD_RENDERER_HPP

#include "gomoku/ABoardView.hpp"
#include <SFML/Graphics.hpp>

enum class CellState {
    Empty = 0,
    Player1 = 1,
    Player2 = 2
};

class GameBoardRenderer {
public:
    GameBoardRenderer(void);
    ~GameBoardRenderer(void);

    void init(void);
    void cleanup(void);
    void render(sf::RenderWindow& window);
    void updateCell(int x, int y, CellState state);

    void setTextures(sf::Texture& boardTexture, sf::Texture& pawn1Texture, sf::Texture& pawn2Texture);

    // Conversion isométrique centrée
    sf::Vector2f isoToScreen(int i, int j, float tileW, float tileH, float centerX, float centerY);

    // Synchronise l'état interne avec un ABoardView (source de vérité)
    void applyBoard(const gomoku::ABoardView& view);

private:
    CellState _board[19][19];

    sf::Sprite* _boardSprite;
    sf::Sprite* _pawn1Sprite;
    sf::Sprite* _pawn2Sprite;
};

#endif