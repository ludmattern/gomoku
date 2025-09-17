#include "gui/ResourceManager.hpp"
#include <iostream>
#include <string>

namespace gomoku::gui {

ResourceManager::ResourceManager(std::string packagePath)
    : texturePath_("assets/textures_pack/" + packagePath + "/")
{
}

ResourceManager::~ResourceManager() = default;

bool ResourceManager::init()
{
    std::cout << "Initializing ResourceManager" << std::endl;

    std::cout << "Texture path: " << texturePath_ << std::endl;

    if (!loadTexture("background", texturePath_ + "Title with bg.png"))
        return false;
    if (!loadTexture("gameBackground", texturePath_ + "background.png"))
        return false;
    if (!loadTexture("board", texturePath_ + "board.png"))
        return false;
    if (!loadTexture("pawn1", texturePath_ + "whitePawn.png"))
        return false;
    if (!loadTexture("pawn2", texturePath_ + "blackPawn.png"))
        return false;
    if (!loadTexture("play_button", texturePath_ + "ui/play_button.png"))
        return false;
    if (!loadTexture("settings_button", texturePath_ + "ui/settings_button.png"))
        return false;
    if (!loadTexture("exit_button", texturePath_ + "ui/exit_button.png"))
        return false;
    if (!loadTexture("vs_player_button", texturePath_ + "ui/vs_player_button.png"))
        return false;
    if (!loadTexture("vs_ai_button", texturePath_ + "ui/vs_ai_button.png"))
        return false;
    if (!loadTexture("back_button", texturePath_ + "ui/back_button.png"))
        return false;
    if (!loadTexture("empty_background", texturePath_ + "background.png"))
        return false;

    // Optional audio assets
    loadSoundOptional("ui_hover", "assets/audio/ui_hover.wav");
    loadSoundOptional("ui_click", "assets/audio/ui_click.wav");
    loadSoundOptional("place_stone_white", "assets/audio/place_white.wav");
    loadSoundOptional("place_stone_black", "assets/audio/place_black.wav");
    loadSoundOptional("capture", "assets/audio/capture.wav");
    loadSoundOptional("win", "assets/audio/win.wav");
    loadSoundOptional("lose", "assets/audio/lose.wav");
    loadSoundOptional("draw", "assets/audio/draw.wav");

    std::cout << "ResourceManager initialized" << std::endl;
    return true;
}

bool ResourceManager::setTexturePackage(const std::string& theme)
{
    std::string newPath = "assets/textures_pack/" + theme + "/";
    // Recharge de manière tolérante avec fallback sur le thème par défaut
    // et sans effacer les textures déjà valides pour éviter les sprites blancs.
    auto themed = [&](const std::string& rel) { return newPath + rel; };
    auto deflt = [&](const std::string& rel) { return std::string("assets/textures_pack/default/") + rel; };

    struct Item { const char* key; const char* rel; } items[] = {
        {"background", "Title with bg.png"},
        {"gameBackground", "background.png"},
        {"board", "board.png"},
        {"pawn1", "whitePawn.png"},
        {"pawn2", "blackPawn.png"},
        {"play_button", "ui/play_button.png"},
        {"settings_button", "ui/settings_button.png"},
        {"exit_button", "ui/exit_button.png"},
        {"vs_player_button", "ui/vs_player_button.png"},
        {"vs_ai_button", "ui/vs_ai_button.png"},
        {"back_button", "ui/back_button.png"},
        {"empty_background", "background.png"},
    };

    bool allOk = true;
    for (const auto& it : items) {
        const std::string themedPath = themed(it.rel);
        const std::string defaultPath = deflt(it.rel);
        // Essaye chemin du thème, sinon fallback défaut, sinon garde l'ancienne texture
        if (!loadTextureIfExists(it.key, themedPath)) {
            if (!loadTextureIfExists(it.key, defaultPath)) {
                std::cerr << "Warning: texture '" << it.key << "' not found in theme nor default. Keeping previous if any." << std::endl;
                allOk = false; // mais on n'invalide pas l'état existant
            }
        }
    }
    texturePath_ = newPath;
    return allOk;
}

void ResourceManager::cleanup()
{
    std::cout << "Cleaning up ResourceManager" << std::endl;
    textures_.clear();
    sounds_.clear();
    std::cout << "ResourceManager cleaned up" << std::endl;
}

sf::Texture& ResourceManager::getTexture(const std::string& name)
{
    auto it = textures_.find(name);
    if (it == textures_.end()) {
        std::cerr << "Texture " << name << " not found" << std::endl;
        throw std::runtime_error("Texture not found");
    }
    return it->second;
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "Failed to load texture " << name << " from " << path << std::endl;
        return false;
    }
    texture.setSmooth(true);
    textures_[name] = std::move(texture);
    std::cout << "Texture " << name << " loaded" << std::endl;
    return true;
}

bool ResourceManager::loadTextureIfExists(const std::string& name, const std::string& path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        return false;
    }
    texture.setSmooth(true);
    textures_[name] = std::move(texture);
    std::cout << "Texture " << name << " loaded" << std::endl;
    return true;
}

bool ResourceManager::hasTexture(const std::string& name) const
{
    return textures_.find(name) != textures_.end();
}

bool ResourceManager::loadSound(const std::string& name, const std::string& path)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        std::cerr << "Failed to load sound " << name << " from " << path << std::endl;
        return false;
    }
    sounds_[name] = std::move(buffer);
    std::cout << "Sound " << name << " loaded" << std::endl;
    return true;
}

bool ResourceManager::loadSoundOptional(const std::string& name, const std::string& path)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        std::cerr << "(Optional) could not load sound " << name << " from " << path << std::endl;
        return false;
    }
    sounds_[name] = std::move(buffer);
    std::cout << "Sound " << name << " loaded" << std::endl;
    return true;
}

bool ResourceManager::hasSound(const std::string& name) const
{
    return sounds_.find(name) != sounds_.end();
}

const sf::SoundBuffer* ResourceManager::getSound(const std::string& name) const
{
    auto it = sounds_.find(name);
    if (it == sounds_.end())
        return nullptr;
    return &it->second;
}

} // namespace gomoku::gui
