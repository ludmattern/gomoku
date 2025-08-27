#include "scene/GameScene.hpp"
#include "scene/MainMenu.hpp"
#include <cmath>
#include <algorithm>

GameScene::GameScene(Context& context, bool vsAi)
	: AScene(context), _vsAi(vsAi)
{
	// Initialisation du bouton Back
	_backButton.setPosition({100, 820});
	_backButton.setSize({300, 70});
	if (_context.ressourceManager && _context.ressourceManager->hasTexture("back_button"))
		_backButton.setTexture(&_context.ressourceManager->getTexture("back_button"));
	_backButton.setScale(1.0f);
	_backButton.setCallback([this]() { onBackClicked(); });

	// Initialisation du renderer de plateau
	if (_context.ressourceManager)
	{
		_boardRenderer.setTextures(
			_context.ressourceManager->getTexture("board"),
			_context.ressourceManager->getTexture("pawn1"),
			_context.ressourceManager->getTexture("pawn2")
		);
	}
}

GameScene::~GameScene(void)
{
	_boardRenderer.cleanup();
}

bool GameScene::handleInput(sf::Event& event)
{
	if (_context.window && _backButton.handleInput(event, *_context.window)) return true;

	// Prévisualisation temporairement désactivée pour debug

	// Placement des pions sur clic souris
	if (_context.window && event.type == sf::Event::MouseButtonPressed)
	{
		auto btn = event.mouseButton.button;
		if (btn == sf::Mouse::Left || btn == sf::Mouse::Right)
		{
			const auto size = _context.window->getSize();
			const float centerX = static_cast<float>(size.x) * 0.5f;
			const float centerY = static_cast<float>(size.y) * 0.5f;

			const int N = 19;
			const int C = (N - 1) / 2;

			const float tileW = std::min(size.x * 0.8f / 18.f, size.y * 0.8f * 2.f / 18.f);
			const float tileH = tileW * 0.5f;

			sf::Vector2f mp = _context.window->mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
			const float X = mp.x - centerX;
			const float Y = mp.y - centerY;

			const float u = (Y / (tileH * 0.5f) + X / (tileW * 0.5f)) * 0.5f;
			const float v = (Y / (tileH * 0.5f) - X / (tileW * 0.5f)) * 0.5f;

			int i = static_cast<int>(std::lround(u)) + C;
			int j = static_cast<int>(std::lround(v)) + C;

			i = std::max(0, std::min(18, i));
			j = std::max(0, std::min(18, j));

			// Validation de proximité
			const float ui = static_cast<float>(i - C);
			const float vj = static_cast<float>(j - C);
			const float snappedX = centerX + (ui - vj) * (tileW * 0.5f);
			const float snappedY = centerY + (ui + vj) * (tileH * 0.5f);
			const float dx = snappedX - mp.x;
			const float dy = snappedY - mp.y;
			const float maxDist = std::min(tileW, tileH) * 0.9f; // Zone cliquable étendue (était 0.35f)

			if ((dx * dx + dy * dy) <= (maxDist * maxDist))
			{
				if (btn == sf::Mouse::Left)
					const_cast<GameBoardRenderer&>(_boardRenderer).updateCell(i, j, CellState::Player1);
				else if (btn == sf::Mouse::Right)
					const_cast<GameBoardRenderer&>(_boardRenderer).updateCell(i, j, CellState::Player2);
			}
		}
		return true;
	}
	return false;
}

void GameScene::update(sf::Time& deltaTime)
{
	_backButton.update(deltaTime);
}

void GameScene::render(sf::RenderTarget& target) const
{
	// Fond de jeu
	if (_context.ressourceManager && _context.ressourceManager->hasTexture("gameBackground"))
	{
		sf::Sprite bg(_context.ressourceManager->getTexture("gameBackground"));
		bg.setScale(sf::Vector2f(1.0f, 1.0f));
		target.draw(bg);
	}
	// Plateau (cible est la fenêtre; cast suffisant ici)
	const_cast<GameBoardRenderer&>(_boardRenderer).render(static_cast<sf::RenderWindow&>(target));
	// UI
	_backButton.render(target);
}

void GameScene::onBackClicked(void)
{
	_context.inGame = false;
	_context.showMainMenu = true;
} 