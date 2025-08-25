#include "scene/MainMenu.hpp"
#include "GameWindow.hpp"
#include <iostream>

MainMenu::MainMenu(Context& context) : AScene(context)
{
	// Bouton "Jouer"
	_playButton.setPosition({86, 539});
	_playButton.setSize({300, 70});
	if (_context.ressourceManager && _context.ressourceManager->hasTexture("play_button"))
	{
		_playButton.setTexture(&_context.ressourceManager->getTexture("play_button"));
	}
	_playButton.setScale(0.775f);
	_playButton.setCallback([this]() { onPlayClicked(); });

	// Bouton "Paramètres"
	_settingsButton.setPosition({537, 539});
	_settingsButton.setSize({300, 70});
	if (_context.ressourceManager && _context.ressourceManager->hasTexture("settings_button"))
	{
		_settingsButton.setTexture(&_context.ressourceManager->getTexture("settings_button"));
	}
	_settingsButton.setScale(0.775f);
	_settingsButton.setCallback([this]() { onSettingsClicked(); });

	// Bouton "Quitter"
	_exitButton.setPosition({995, 539});
	_exitButton.setSize({300, 70});
	if (_context.ressourceManager && _context.ressourceManager->hasTexture("exit_button"))
	{
		_exitButton.setTexture(&_context.ressourceManager->getTexture("exit_button"));
	}
	_exitButton.setScale(0.775f);
	_exitButton.setCallback([this]() { onExitClicked(); });
}

MainMenu::~MainMenu(void)
{
}

bool MainMenu::handleInput(sf::Event& event)
{
	return (_context.window && _playButton.handleInput(event, *_context.window))
		|| (_context.window && _settingsButton.handleInput(event, *_context.window))
		|| (_context.window && _exitButton.handleInput(event, *_context.window));
}

void MainMenu::update(sf::Time& deltaTime)
{
	_playButton.update(deltaTime);
	_settingsButton.update(deltaTime);
	_exitButton.update(deltaTime);
}

void MainMenu::render(sf::RenderTarget& target) const
{
	_playButton.render(target);
	_settingsButton.render(target);
	_exitButton.render(target);
}

void MainMenu::onPlayClicked(void)
{
	_context.inGame = false;
	_context.showGameSelectMenu = true;
}

void MainMenu::onSettingsClicked(void)
{
}

void MainMenu::onExitClicked(void)
{
	_context.shouldQuit = true;
}