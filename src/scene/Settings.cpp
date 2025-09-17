#include "scene/Settings.hpp"

namespace gomoku::scene {

SettingsScene::SettingsScene(Context& ctx)
    : AScene(ctx)
{
    // Boutons de thème
    defaultBtn_.setPosition({ 150, 300 });
    defaultBtn_.setSize({ 300, 70 });
    defaultBtn_.setScale(1.0f);
    defaultBtn_.setCallback([this]() { applyTheme("default"); });
    defaultBtn_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });
    
    halloweenBtn_.setPosition({ 150, 480 });
    halloweenBtn_.setSize({ 300, 70 });
    halloweenBtn_.setScale(1.0f);
    halloweenBtn_.setCallback([this]() { applyTheme("halloween"); });
    halloweenBtn_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });

    pastelBtn_.setPosition({ 150, 570 });
    pastelBtn_.setSize({ 300, 70 });
    pastelBtn_.setScale(1.0f);
    pastelBtn_.setCallback([this]() { applyTheme("pastel"); });
    pastelBtn_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });

    // Bouton retour
    backBtn_.setPosition({ 100, 820 });
    backBtn_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("back_button"))
        backBtn_.setTexture(&context_.resourceManager->getTexture("back_button"));
    backBtn_.setScale(1.0f);
    backBtn_.setCallback([this]() { onBackClicked(); });
    backBtn_.setHoverCallback([this]() { playSfx("ui_hover", 40.f); });
}

void SettingsScene::update(sf::Time& deltaTime)
{
    defaultBtn_.update(deltaTime);
    darkBtn_.update(deltaTime);
    halloweenBtn_.update(deltaTime);
    pastelBtn_.update(deltaTime);
    backBtn_.update(deltaTime);
}

void SettingsScene::render(sf::RenderTarget& target) const
{
    defaultBtn_.render(target);
    darkBtn_.render(target);
    halloweenBtn_.render(target);
    pastelBtn_.render(target);
    backBtn_.render(target);
}

void SettingsScene::onThemeChanged()
{
    if (!context_.resourceManager)
        return;
    // Rebind back button (les autres sont rectangles sans texture par défaut)
    if (context_.resourceManager->hasTexture("back_button"))
        backBtn_.setTexture(&context_.resourceManager->getTexture("back_button"));
}

bool SettingsScene::handleInput(sf::Event& event)
{
    if (context_.window && defaultBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", 80.f); return true; }
    if (context_.window && darkBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", 80.f); return true; }
    if (context_.window && halloweenBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", 80.f); return true; }
    if (context_.window && pastelBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", 80.f); return true; }
    if (context_.window && backBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", 80.f); return true; }
    return false;
}

void SettingsScene::applyTheme(const std::string& themeName)
{
    if (!context_.resourceManager)
        return;
    if (context_.resourceManager->setTexturePackage(themeName)) {
        context_.theme = themeName;
        context_.themeChanged = true;
        // Si en jeu on rebindrait les textures; ici on reste dans Settings
    }
}

void SettingsScene::onBackClicked()
{
    context_.showSettingsMenu = false;
    context_.showMainMenu = true;
}

} // namespace gomoku::scene


