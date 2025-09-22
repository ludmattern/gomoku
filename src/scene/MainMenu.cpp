#include "scene/MainMenu.hpp"
#include <iostream>
#include "audio/Volumes.hpp"

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
    playButton_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });

    settingsButton_.setPosition({ 693, 696 });
    settingsButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("settings_button"))
        settingsButton_.setTexture(&context_.resourceManager->getTexture("settings_button"));
    settingsButton_.setScale(1.0f);
    settingsButton_.setCallback([this]() { onSettingsClicked(); });
    settingsButton_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });

    exitButton_.setPosition({ 1284, 695.5f });
    exitButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("exit_button"))
        exitButton_.setTexture(&context_.resourceManager->getTexture("exit_button"));
    exitButton_.setScale(1.0f);
    exitButton_.setCallback([this]() { onExitClicked(); });
    exitButton_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });
}

MainMenu::~MainMenu() = default;

bool MainMenu::handleInput(sf::Event& event)
{
    bool consumed = false;
    if (context_.window) {
        auto handleBtn = [&](gomoku::ui::Button& btn) {
            bool c = btn.handleInput(event, *context_.window);
            if (event.type == sf::Event::MouseButtonReleased && c)
                playSfx("ui_click", BUTTON_VOLUME);
            return c;
        };
        consumed = handleBtn(playButton_) || handleBtn(settingsButton_) || handleBtn(exitButton_);
    }
    return consumed;
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

void MainMenu::onThemeChanged()
{
    if (!context_.resourceManager)
        return;
    if (context_.resourceManager->hasTexture("play_button"))
        playButton_.setTexture(&context_.resourceManager->getTexture("play_button"));
    if (context_.resourceManager->hasTexture("settings_button"))
        settingsButton_.setTexture(&context_.resourceManager->getTexture("settings_button"));
    if (context_.resourceManager->hasTexture("exit_button"))
        exitButton_.setTexture(&context_.resourceManager->getTexture("exit_button"));
}

void MainMenu::onPlayClicked()
{
    context_.inGame = false;
    context_.showGameSelectMenu = true;
    // keep menu music by default
}

void MainMenu::onSettingsClicked()
{
    context_.showSettingsMenu = true;
}

void MainMenu::onExitClicked()
{
    context_.shouldQuit = true;
}

} // namespace gomoku::scene