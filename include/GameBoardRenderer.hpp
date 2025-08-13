#ifndef GAME_BOARD_RENDERER_HPP
# define GAME_BOARD_RENDERER_HPP

#include <SFML/Graphics.hpp>

enum class CellState
{
    Empty = 0,
    Player1 = 1,
    Player2 = 2
};

class GameBoardRenderer
{
    public:
        GameBoardRenderer(void);
        ~GameBoardRenderer(void);

        void init(void);
        void cleanup(void);
        void render(sf::RenderWindow& window);
        void updateCell(int x, int y, CellState state);
        
        // Conversion isométrique centrée
        sf::Vector2f isoToScreen(int i, int j, float tileW, float tileH, float centerX, float centerY);

    private:
        CellState _board[19][19];
};

#endif