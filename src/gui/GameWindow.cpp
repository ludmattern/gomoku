#include "GameWindow.hpp"
#include <iostream>
#include <cmath>
#include "GameBoardRenderer.hpp"
#include "scene/Context.hpp"
#include "scene/MainMenu.hpp"

GameWindow::GameWindow(void) : _ressourceManager("default"), _backgroundSprite(nullptr)
{
	_isRunning = false;
	_cleaned = false;
	init();
}

GameWindow::~GameWindow(void)
{
	// Ne rien faire ici; cleanup() est déjà appelé depuis run()
}

bool GameWindow::isRunning(void)
{
	return _isRunning;
}

void GameWindow::init(void)
{	
	_window = sf::RenderWindow(
		sf::VideoMode({1920, 1080}),
		"Gomoku"
	);
	_window.setFramerateLimit(60);

	_context = Context();
	_context.window = &_window;
	_context.ressourceManager = &_ressourceManager;
	
	if (!_ressourceManager.init())
	{
		std::cerr << "Failed to initialize RessourceManager" << std::endl;
		return;
	}
	
	// Background
	_backgroundSprite = new sf::Sprite(_ressourceManager.getTexture("background"));
	
	_boardRenderer.setTextures(
		_ressourceManager.getTexture("board"),
		_ressourceManager.getTexture("pawn1"),
		_ressourceManager.getTexture("pawn2")
	);

	// Scène: démarrer sur le menu principal
	_currentScene = std::make_unique<MainMenu>(_context);
	_currentScene->onEnter();

	_isRunning = true;
}

void GameWindow::handleEvents(void)
{
	while (auto event = _window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			_context.shouldQuit = true;
			return;
		}
		if (event->is<sf::Event::KeyPressed>())
		{
			auto key = event->getIf<sf::Event::KeyPressed>()->code;
			if (key == sf::Keyboard::Key::Escape)
			{
				_context.shouldQuit = true;
				return;
			}
		}

		if (_currentScene)
		{
			sf::Event evtCopy = *event;
			_currentScene->handleInput(evtCopy);
			if (_context.shouldQuit)
				return;
		}

		// Interaction plateau en mode jeu
		if (_context.inGame && event->is<sf::Event::MouseButtonPressed>())
		{
			auto pressed = event->getIf<sf::Event::MouseButtonPressed>();
			auto btn = pressed->button;
			if (btn == sf::Mouse::Button::Left || btn == sf::Mouse::Button::Right)
			{
				const auto size = _window.getSize();
				const float centerX = static_cast<float>(size.x) * 0.5f;
				const float centerY = static_cast<float>(size.y) * 0.5f;

				const int N = 19;
				const int C = (N - 1) / 2;

				const float tileW = std::min(size.x * 0.8f / 18.f, size.y * 0.8f * 2.f / 18.f);
				const float tileH = tileW * 0.5f;

				sf::Vector2f mp = _window.mapPixelToCoords({pressed->position.x, pressed->position.y});
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
				const float maxDist = std::min(tileW, tileH) * 0.35f;

				if ((dx * dx + dy * dy) <= (maxDist * maxDist))
				{
					if (btn == sf::Mouse::Button::Left)
						_boardRenderer.updateCell(i, j, CellState::Player1);
					else if (btn == sf::Mouse::Button::Right)
						_boardRenderer.updateCell(i, j, CellState::Player2);
				}
			}
		}
	}
}

void GameWindow::cleanup(void)
{
	if (_cleaned)
		return;
	_cleaned = true;
	_isRunning = false;

	if (_currentScene)
	{
		_currentScene->onExit();
		_currentScene.reset();
	}

	delete _backgroundSprite;
	_backgroundSprite = nullptr;

	_boardRenderer.cleanup();
	_ressourceManager.cleanup();

	if (_window.isOpen())
		_window.close();
}

void GameWindow::render(void)
{
	if (!_window.isOpen()) return;
	_window.clear(sf::Color::Black);
	
	if (_backgroundSprite)
	{
		_window.draw(*_backgroundSprite);
	}
	
	if (_context.inGame)
	{
		renderBoard();
	}
	else if (_currentScene)
	{
		_currentScene->render(_window);
	}
	
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
		_deltaTime = _clock.restart();
		handleEvents();
		if (!_isRunning)
			break;
		if (_context.inGame && _currentScene)
		{
			_currentScene->onExit();
			_currentScene.reset();
		}
		if (_context.shouldQuit)
		{
			cleanup();
			std::exit(0);
		}
		if (_currentScene)
		{
			_currentScene->update(_deltaTime);
		}
		render();
	}
}