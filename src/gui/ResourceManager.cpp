#include "gui/ResourceManager.hpp"
#include <iostream>
#include <string>

ResourceManager::ResourceManager(std::string packagePath)
    : TEXTURE_PATH("assets/textures_pack/" + packagePath + "/")
{
}

ResourceManager::~ResourceManager(void)
{
}

bool ResourceManager::init(void)
{
    std::cout << "Initializing ResourceManager" << std::endl;

    if (!loadTexture("background", "assets/Title with bg.png")) {
        std::cerr << "Failed to load background texture" << std::endl;
        return false;
    }
    if (!loadTexture("gameBackground", "assets/background.png")) {
        std::cerr << "Failed to load game background texture" << std::endl;
        return false;
    }
    if (!loadTexture("board", TEXTURE_PATH + "board.png")) {
        std::cerr << "Failed to load board texture" << std::endl;
        return false;
    }
    if (!loadTexture("pawn1", TEXTURE_PATH + "whitePawn.png")) {
        std::cerr << "Failed to load pawn1 texture" << std::endl;
        return false;
    }
    if (!loadTexture("pawn2", TEXTURE_PATH + "blackPawn.png")) {
        std::cerr << "Failed to load pawn2 texture" << std::endl;
        return false;
    }
    if (!loadTexture("play_button", "assets/ui/play_button.png")) {
        std::cerr << "Failed to load play_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("settings_button", "assets/ui/settings_button.png")) {
        std::cerr << "Failed to load settings_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("exit_button", "assets/ui/exit_button.png")) {
        std::cerr << "Failed to load exit_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("vs_player_button", "assets/ui/vs_player_button.png")) {
        std::cerr << "Failed to load vs_player_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("vs_ai_button", "assets/ui/vs_ai_button.png")) {
        std::cerr << "Failed to load vs_ai_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("back_button", "assets/ui/back_button.png")) {
        std::cerr << "Failed to load back_button texture" << std::endl;
        return false;
    }
    if (!loadTexture("empty_background", "assets/background.png")) {
        std::cerr << "Failed to load empty_background texture" << std::endl;
        return false;
    }

    std::cout << "ResourceManager initialized" << std::endl;
    return true;
}

void ResourceManager::cleanup(void)
{
    std::cout << "Cleaning up ResourceManager" << std::endl;
    _textures.clear();
    std::cout << "ResourceManager cleaned up" << std::endl;
}

sf::Texture& ResourceManager::getTexture(const std::string& name)
{
    auto it = _textures.find(name);
    if (it == _textures.end()) {
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
    _textures[name] = std::move(texture);
    std::cout << "Texture " << name << " loaded" << std::endl;
    return true;
}

bool ResourceManager::hasTexture(const std::string& name) const
{
    return _textures.find(name) != _textures.end();
}
