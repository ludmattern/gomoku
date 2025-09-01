#pragma once

#include "scene/AScene.hpp"
#include "ui/Button.hpp"
#include "GameBoardRenderer.hpp"
#include "gomoku/GameSession.hpp"
#include "gomoku/Types.hpp"




class GameScene : public AScene
{
	public:
		explicit GameScene(Context& context, bool vsAi);
		~GameScene(void);

		bool handleInput(sf::Event& event) override;
		void update(sf::Time& deltaTime) override;
		void render(sf::RenderTarget& target) const override;

	private:
		void onBackClicked(void);

		bool _vsAi;
		Button _backButton;
		GameBoardRenderer _boardRenderer;
		gomoku::GameSession _gameSession;
		gomoku::RuleSet _rules;
	}; 