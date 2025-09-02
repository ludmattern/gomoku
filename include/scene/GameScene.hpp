#pragma once

#include "scene/AScene.hpp"
#include "ui/Button.hpp"
#include "GameBoardRenderer.hpp"
#include "gomoku/GameSession.hpp"
#include "gomoku/Types.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class GameScene : public AScene
{
public:
	explicit GameScene(Context &context, bool vsAi);
	~GameScene(void);

	bool handleInput(sf::Event &event) override;
	void update(sf::Time &deltaTime) override;
	void render(sf::RenderTarget &target) const override;

private:
	void onBackClicked(void);

	bool _vsAi;
	Button _backButton;
	GameBoardRenderer _boardRenderer;
	gomoku::GameSession _gameSession;
	gomoku::RuleSet _rules;

	// HUD / overlay
	mutable sf::Font _font;
	mutable bool _fontOk = false;
	mutable sf::Text _hudText;
	mutable int _lastAiMs = -1;
	bool _pendingAi = false;
	int _aiBudgetMs = 450;
	mutable bool _framePresented = false; // set true by render() once a frame was displayed
	// Illegal move message
	std::string _illegalMsg;
	mutable sf::Clock _illegalClock;
	mutable sf::Text _msgText;
};