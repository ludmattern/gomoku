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

    if (!loadTexture("background", "assets/Title with bg.png"))
        return false;
    if (!loadTexture("gameBackground", "assets/background.png"))
        return false;
    if (!loadTexture("board", texturePath_ + "board.png"))
        return false;
    if (!loadTexture("pawn1", texturePath_ + "whitePawn.png"))
        return false;
    if (!loadTexture("pawn2", texturePath_ + "blackPawn.png"))
        return false;
    if (!loadTexture("play_button", "assets/ui/play_button.png"))
        return false;
    if (!loadTexture("settings_button", "assets/ui/settings_button.png"))
        return false;
    if (!loadTexture("exit_button", "assets/ui/exit_button.png"))
        return false;
    if (!loadTexture("vs_player_button", "assets/ui/vs_player_button.png"))
        return false;
    if (!loadTexture("vs_ai_button", "assets/ui/vs_ai_button.png"))
        return false;
    if (!loadTexture("back_button", "assets/ui/back_button.png"))
        return false;
    if (!loadTexture("empty_background", "assets/background.png"))
        return false;

    std::cout << "ResourceManager initialized" << std::endl;
    return true;
}

void ResourceManager::cleanup()
{
    std::cout << "Cleaning up ResourceManager" << std::endl;
    textures_.clear();
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

bool ResourceManager::hasTexture(const std::string& name) const
{
    return textures_.find(name) != textures_.end();
}

} // namespace gomoku::gui
