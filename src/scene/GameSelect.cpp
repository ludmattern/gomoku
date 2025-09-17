#include "scene/GameSelect.hpp"

namespace gomoku::scene {

GameSelectScene::GameSelectScene(Context& ctx)
    : AScene(ctx)
{
    playerVsPlayerButton_.setPosition({ 111, 696 });
    playerVsPlayerButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("vs_player_button"))
        playerVsPlayerButton_.setTexture(&context_.resourceManager->getTexture("vs_player_button"));
    playerVsPlayerButton_.setScale(1.0f);
    playerVsPlayerButton_.setCallback([this]() { onPlayerVsPlayerClicked(); });
    playerVsPlayerButton_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });

    playerVsBotButton_.setPosition({ 693, 696 });
    playerVsBotButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("vs_ai_button"))
        playerVsBotButton_.setTexture(&context_.resourceManager->getTexture("vs_ai_button"));
    playerVsBotButton_.setScale(1.0f);
    playerVsBotButton_.setCallback([this]() { onPlayerVsBotClicked(); });
    playerVsBotButton_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });

    backButton_.setPosition({ 1284, 695.5f });
    backButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("back_button"))
        backButton_.setTexture(&context_.resourceManager->getTexture("back_button"));
    backButton_.setScale(1.0f);
    backButton_.setCallback([this]() { onBackClicked(); });
    backButton_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });
}

void GameSelectScene::update(sf::Time& deltaTime)
{
    playerVsPlayerButton_.update(deltaTime);
    playerVsBotButton_.update(deltaTime);
    backButton_.update(deltaTime);
}

void GameSelectScene::render(sf::RenderTarget& target) const
{
    playerVsPlayerButton_.render(target);
    playerVsBotButton_.render(target);
    backButton_.render(target);
}

void GameSelectScene::onThemeChanged()
{
    if (!context_.resourceManager)
        return;
    if (context_.resourceManager->hasTexture("vs_player_button"))
        playerVsPlayerButton_.setTexture(&context_.resourceManager->getTexture("vs_player_button"));
    if (context_.resourceManager->hasTexture("vs_ai_button"))
        playerVsBotButton_.setTexture(&context_.resourceManager->getTexture("vs_ai_button"));
    if (context_.resourceManager->hasTexture("back_button"))
        backButton_.setTexture(&context_.resourceManager->getTexture("back_button"));
}

bool GameSelectScene::handleInput(sf::Event& event)
{
    bool consumed = false;
    if (context_.window) {
        auto handleBtn = [&](gomoku::ui::Button& btn) {
            bool c = btn.handleInput(event, *context_.window);
            if (event.type == sf::Event::MouseButtonReleased && c)
                playSfx("ui_click", 80.f);
            return c;
        };
        consumed = handleBtn(playerVsPlayerButton_) || handleBtn(playerVsBotButton_) || handleBtn(backButton_);
    }
    return consumed;
}

void GameSelectScene::onPlayerVsPlayerClicked()
{
    context_.vsAi = false;
    context_.inGame = true;
    playMusic("assets/audio/ingame_theme.ogg", true, 10.f);
}

void GameSelectScene::onPlayerVsBotClicked()
{
    context_.vsAi = true;
    context_.inGame = true;
    playMusic("assets/audio/ingame_theme.ogg", true, 10.f);
}

void GameSelectScene::onBackClicked()
{
    context_.showGameSelectMenu = false;
    context_.inGame = false;
    context_.showMainMenu = true;
}

} // namespace gomoku::scene