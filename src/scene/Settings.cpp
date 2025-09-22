#include "scene/Settings.hpp"
#include <iostream>
#include "audio/Volumes.hpp"
#include "util/Preferences.hpp"

namespace gomoku::scene {

SettingsScene::SettingsScene(Context& ctx)
    : AScene(ctx)
{
    // Boutons de thème
    defaultBtn_.setPosition({ 1020, 580 });
    defaultBtn_.setSize({ 300, 70 });
    defaultBtn_.setScale(0.2f);
    defaultBtn_.setCallback([this]() { applyTheme("default"); });
    defaultBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });
    if (context_.resourceManager && context_.resourceManager->hasTexture("default_theme_button"))
        defaultBtn_.setTexture(&context_.resourceManager->getTexture("default_theme_button"));
    
    halloweenBtn_.setPosition({ 1150, 580 });
    halloweenBtn_.setSize({ 300, 70 });
    halloweenBtn_.setScale(0.2f);
    halloweenBtn_.setCallback([this]() { applyTheme("halloween"); });
    halloweenBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });
    if (context_.resourceManager && context_.resourceManager->hasTexture("halloween_theme_button"))
        halloweenBtn_.setTexture(&context_.resourceManager->getTexture("halloween_theme_button"));

    pastelBtn_.setPosition({ 1280, 580 });
    pastelBtn_.setSize({ 300, 70 });
    pastelBtn_.setScale(0.2f);
    pastelBtn_.setCallback([this]() { applyTheme("pastel"); });
    pastelBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });
    if (context_.resourceManager && context_.resourceManager->hasTexture("pastel_theme_button"))
        pastelBtn_.setTexture(&context_.resourceManager->getTexture("pastel_theme_button"));

    // Bouton retour
    backBtn_.setPosition({ 695, 730 });
    backBtn_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("back_button"))
        backBtn_.setTexture(&context_.resourceManager->getTexture("back_button"));
    backBtn_.setScale(1.0f);
    backBtn_.setCallback([this]() { onBackClicked(); });
    backBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });

    // Bouton SFX toggle
    sfxToggleBtn_.setPosition({ 1150, 340 });
    sfxToggleBtn_.setSize({ 10, 10 });
    sfxToggleBtn_.setScale(0.15f);
    sfxToggleBtn_.setCallback([this]() { toggleSfx(); });
    sfxToggleBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });

    // Bouton Musique toggle
    musicToggleBtn_.setPosition({ 1150, 460 });
    musicToggleBtn_.setSize({ 10, 10 });
    musicToggleBtn_.setScale(0.15f);
    musicToggleBtn_.setCallback([this]() { toggleMusic(); });
    musicToggleBtn_.setHoverCallback([this]() { playSfx("ui_hover", UI_HOVER_VOLUME); });

    // Texture initiale selon état
    if (context_.resourceManager) {
        const char* onKey = "sound_on";
        const char* offKey = "sound_off";
        if (context_.resourceManager->hasTexture(context_.sfxEnabled ? onKey : offKey))
            sfxToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.sfxEnabled ? onKey : offKey));
        if (context_.resourceManager->hasTexture(context_.musicEnabled ? onKey : offKey))
            musicToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.musicEnabled ? onKey : offKey));
    }
}

void SettingsScene::update(sf::Time& deltaTime)
{
    defaultBtn_.update(deltaTime);
    darkBtn_.update(deltaTime);
    halloweenBtn_.update(deltaTime);
    pastelBtn_.update(deltaTime);
    backBtn_.update(deltaTime);
    sfxToggleBtn_.update(deltaTime);
    musicToggleBtn_.update(deltaTime);
}

void SettingsScene::render(sf::RenderTarget& target) const
{
    defaultBtn_.render(target);
    darkBtn_.render(target);
    halloweenBtn_.render(target);
    pastelBtn_.render(target);
    backBtn_.render(target);
    sfxToggleBtn_.render(target);
    musicToggleBtn_.render(target);
}

void SettingsScene::onThemeChanged()
{
    if (!context_.resourceManager)
        return;
    // Rebind back button (les autres sont rectangles sans texture par défaut)
    if (context_.resourceManager->hasTexture("back_button"))
        backBtn_.setTexture(&context_.resourceManager->getTexture("back_button"));
    if (context_.resourceManager->hasTexture("default_theme_button"))
        defaultBtn_.setTexture(&context_.resourceManager->getTexture("default_theme_button"));
    if (context_.resourceManager->hasTexture("halloween_theme_button"))
        halloweenBtn_.setTexture(&context_.resourceManager->getTexture("halloween_theme_button"));
    if (context_.resourceManager->hasTexture("pastel_theme_button"))
        pastelBtn_.setTexture(&context_.resourceManager->getTexture("pastel_theme_button"));
    // rafraîchit icônes (thémées)
    const char* onKey = "sound_on";
    const char* offKey = "sound_off";
    if (context_.resourceManager->hasTexture(context_.sfxEnabled ? onKey : offKey))
        sfxToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.sfxEnabled ? onKey : offKey));
    if (context_.resourceManager->hasTexture(context_.musicEnabled ? onKey : offKey))
        musicToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.musicEnabled ? onKey : offKey));
}

bool SettingsScene::handleInput(sf::Event& event)
{
    if (context_.window && defaultBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && darkBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && halloweenBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && pastelBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && backBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && sfxToggleBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    if (context_.window && musicToggleBtn_.handleInput(event, *context_.window)) { playSfx("ui_click", BUTTON_VOLUME); return true; }
    return false;
}

void SettingsScene::applyTheme(const std::string& themeName)
{
    if (!context_.resourceManager)
        return;
    const bool texOk = context_.resourceManager->setTexturePackage(themeName);
    const bool audOk = context_.resourceManager->setAudioPackage(themeName);

    if (audOk && texOk) {
        context_.theme = themeName;
        context_.themeChanged = true;
        std::string musicPath = std::string("assets/audio/") + themeName + "/menu_theme.ogg";
        playMusic(musicPath.c_str(), true, MUSIC_VOLUME);
        // persiste préférences
        gomoku::util::PreferencesData prefs;
        prefs.theme = context_.theme;
        prefs.sfxEnabled = context_.sfxEnabled;
        prefs.musicEnabled = context_.musicEnabled;
        gomoku::util::Preferences::save(prefs);
        std::cout << "Theme applied: " << themeName << std::endl;
    } else {
        std::cerr << "Failed to apply theme " << themeName << std::endl;
    }
}

void SettingsScene::onBackClicked()
{
    context_.showSettingsMenu = false;
    context_.showMainMenu = true;
}

void SettingsScene::toggleSfx()
{
    context_.sfxEnabled = !context_.sfxEnabled;
    {
        gomoku::util::PreferencesData prefs;
        prefs.theme = context_.theme;
        prefs.sfxEnabled = context_.sfxEnabled;
        prefs.musicEnabled = context_.musicEnabled;
        gomoku::util::Preferences::save(prefs);
    }
    const char* onKey = "sound_on";
    const char* offKey = "sound_off";
    if (context_.resourceManager && context_.resourceManager->hasTexture(context_.sfxEnabled ? onKey : offKey))
        sfxToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.sfxEnabled ? onKey : offKey));
}

void SettingsScene::toggleMusic()
{
    context_.musicEnabled = !context_.musicEnabled;
    if (context_.music) {
        if (context_.musicEnabled) {
            context_.music->setVolume(std::max(0.f, std::min(100.f, context_.musicVolume)));
            if (context_.music->getStatus() != sf::Music::Status::Playing)
                context_.music->play();
        } else {
            context_.music->setVolume(0.f);
        }
    }
    {
        gomoku::util::PreferencesData prefs;
        prefs.theme = context_.theme;
        prefs.sfxEnabled = context_.sfxEnabled;
        prefs.musicEnabled = context_.musicEnabled;
        gomoku::util::Preferences::save(prefs);
    }
    const char* onKey = "sound_on";
    const char* offKey = "sound_off";
    if (context_.resourceManager && context_.resourceManager->hasTexture(context_.musicEnabled ? onKey : offKey))
        musicToggleBtn_.setTexture(&context_.resourceManager->getTexture(context_.musicEnabled ? onKey : offKey));
}

} // namespace gomoku::scene


