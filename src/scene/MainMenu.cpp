#include "scene/MainMenu.hpp"
#include <iostream>

namespace gomoku::scene {

MainMenu::MainMenu(Context& context)
    : AScene(context)
{
    std::cout << "[MainMenu] ctor" << std::endl;
    playButton_.setPosition({ 111, 696 });
    playButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("play_button"))
        playButton_.setTexture(&context_.resourceManager->getTexture("play_button"));
    playButton_.setScale(1.0f);
    playButton_.setCallback([this]() { onPlayClicked(); });

    settingsButton_.setPosition({ 693, 696 });
    settingsButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("settings_button"))
        settingsButton_.setTexture(&context_.resourceManager->getTexture("settings_button"));
    settingsButton_.setScale(1.0f);
    settingsButton_.setCallback([this]() { onSettingsClicked(); });

    exitButton_.setPosition({ 1284, 695.5f });
    exitButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("exit_button"))
        exitButton_.setTexture(&context_.resourceManager->getTexture("exit_button"));
    exitButton_.setScale(1.0f);
    exitButton_.setCallback([this]() { onExitClicked(); });
}

MainMenu::~MainMenu() = default;

bool MainMenu::handleInput(sf::Event& event)
{
    return (context_.window && playButton_.handleInput(event, *context_.window)) || (context_.window && settingsButton_.handleInput(event, *context_.window)) || (context_.window && exitButton_.handleInput(event, *context_.window));
}

void MainMenu::update(sf::Time& deltaTime)
{
    playButton_.update(deltaTime);
    settingsButton_.update(deltaTime);
    exitButton_.update(deltaTime);
}

void MainMenu::render(sf::RenderTarget& target) const
{
    playButton_.render(target);
    settingsButton_.render(target);
    exitButton_.render(target);
}

void MainMenu::onPlayClicked()
{
    context_.inGame = false;
    context_.showGameSelectMenu = true;
}

void MainMenu::onSettingsClicked() { }

void MainMenu::onExitClicked()
{
    context_.shouldQuit = true;
}

} // namespace gomoku::scene
