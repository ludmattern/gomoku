#include "scene/MainMenu.hpp"
#include "GameWindow.hpp"
#include <iostream>

MainMenu::MainMenu(Context& context) : AScene(context)
{
	// Bouton "Jouer"
	_playButton.setText("Play");
	_playButton.setPosition({200, 550});
	_playButton.setSize({300, 70});
	_playButton.setCornerRadius(0.f);
	_playButton.setCallback([this]() { onPlayClicked(); });

	// Bouton "Paramètres"
	_settingsButton.setText("Settings");
	_settingsButton.setPosition({600, 550});
	_settingsButton.setSize({300, 70});
	_settingsButton.setCornerRadius(0.f);
	_settingsButton.setCallback([this]() { onSettingsClicked(); });

	// Bouton "Quitter"
	_exitButton.setText("Exit");
	_exitButton.setPosition({600, 700});
	_exitButton.setSize({300, 70});
	_exitButton.setCornerRadius(0.f);
	_exitButton.setCallback([this]() { onExitClicked(); });
}

MainMenu::~MainMenu(void)
{
}

void MainMenu::onEnter(void)
{
}

void MainMenu::onExit(void)
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
	_context.inGame = true;
}

void MainMenu::onSettingsClicked(void)
{
}

void MainMenu::onExitClicked(void)
{
	_context.shouldQuit = true;
}