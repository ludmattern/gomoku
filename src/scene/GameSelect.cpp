#include "scene/GameSelect.hpp"

GameSelectScene::GameSelectScene(Context& ctx) : AScene(ctx) {
    // Configuration du bouton "Joueur vs Joueur"
    _playerVsPlayerButton.setText("Joueur vs Joueur");
    _playerVsPlayerButton.setPosition({400, 250});
    _playerVsPlayerButton.setCallback([this]() { onPlayerVsPlayerClicked(); });

    // Configuration du bouton "Joueur vs Bot"
    _playerVsBotButton.setText("Joueur vs Bot");
    _playerVsBotButton.setPosition({400, 350});
    _playerVsBotButton.setCallback([this]() { onPlayerVsBotClicked(); });

    // Configuration du bouton "Retour"
    _backButton.setText("Retour");
    _backButton.setPosition({400, 450});
    _backButton.setCallback([this]() { onBackClicked(); });
}

void GameSelectScene::onEnter(void) {
    // Initialisation de la scène
}

void GameSelectScene::onExit(void) {
    // Nettoyage si nécessaire
}

void GameSelectScene::update(sf::Time& deltaTime) {
    _playerVsPlayerButton.update(deltaTime);
    _playerVsBotButton.update(deltaTime);
    _backButton.update(deltaTime);
}

void GameSelectScene::render(sf::RenderTarget& target) const {
    _playerVsPlayerButton.render(target);
    _playerVsBotButton.render(target);
    _backButton.render(target);
}

bool GameSelectScene::handleInput(sf::Event& event) {
    if (_context.window && _playerVsPlayerButton.handleInput(event, *_context.window)) return true;
    if (_context.window && _playerVsBotButton.handleInput(event, *_context.window)) return true;
    if (_context.window && _backButton.handleInput(event, *_context.window)) return true;
    return false;
}

void GameSelectScene::onPlayerVsPlayerClicked() {
    // TODO: lancer le jeu (SceneManager non intégré pour l'instant)
}

void GameSelectScene::onPlayerVsBotClicked() {
    // TODO: lancer le jeu (SceneManager non intégré pour l'instant)
}

void GameSelectScene::onBackClicked() {
    // TODO: retour au menu principal (SceneManager non intégré pour l'instant)
}