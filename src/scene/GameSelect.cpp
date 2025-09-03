#include "scene/GameSelect.hpp"
#include "scene/MainMenu.hpp"

GameSelectScene::GameSelectScene(Context& ctx)
    : AScene(ctx)
{
    _playerVsPlayerButton.setPosition({ 111, 696 });
    _playerVsPlayerButton.setSize({ 300, 70 });
    if (_context.ressourceManager && _context.ressourceManager->hasTexture("vs_player_button"))
        _playerVsPlayerButton.setTexture(&_context.ressourceManager->getTexture("vs_player_button"));
    _playerVsPlayerButton.setScale(1.0f);
    _playerVsPlayerButton.setCallback([this]() { onPlayerVsPlayerClicked(); });

    _playerVsBotButton.setPosition({ 693, 696 });
    _playerVsBotButton.setSize({ 300, 70 });
    if (_context.ressourceManager && _context.ressourceManager->hasTexture("vs_ai_button"))
        _playerVsBotButton.setTexture(&_context.ressourceManager->getTexture("vs_ai_button"));
    _playerVsBotButton.setScale(1.0f);
    _playerVsBotButton.setCallback([this]() { onPlayerVsBotClicked(); });

    _backButton.setPosition({ 1284, 695.5 });
    _backButton.setSize({ 300, 70 });
    if (_context.ressourceManager && _context.ressourceManager->hasTexture("back_button"))
        _backButton.setTexture(&_context.ressourceManager->getTexture("back_button"));
    _backButton.setScale(1.0f);
    _backButton.setCallback([this]() { onBackClicked(); });
}

void GameSelectScene::update(sf::Time& deltaTime)
{
    _playerVsPlayerButton.update(deltaTime);
    _playerVsBotButton.update(deltaTime);
    _backButton.update(deltaTime);
}

void GameSelectScene::render(sf::RenderTarget& target) const
{
    _playerVsPlayerButton.render(target);
    _playerVsBotButton.render(target);
    _backButton.render(target);
}

bool GameSelectScene::handleInput(sf::Event& event)
{
    if (_context.window && _playerVsPlayerButton.handleInput(event, *_context.window))
        return true;
    if (_context.window && _playerVsBotButton.handleInput(event, *_context.window))
        return true;
    if (_context.window && _backButton.handleInput(event, *_context.window))
        return true;
    return false;
}

void GameSelectScene::onPlayerVsPlayerClicked()
{
    _context.vsAi = false;
    _context.inGame = true;
}

void GameSelectScene::onPlayerVsBotClicked()
{
    _context.vsAi = true;
    _context.inGame = true;
}

void GameSelectScene::onBackClicked()
{
    _context.showGameSelectMenu = false;
    _context.inGame = false;
    _context.showMainMenu = true;
}