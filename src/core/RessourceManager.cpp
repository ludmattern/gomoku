#include "core/RessourceManager.hpp"
#include <iostream>
#include <string>

RessourceManager::RessourceManager(std::string packagePath) : TEXTURE_PATH("assets/textures_pack/" + packagePath + "/")
{
}

RessourceManager::~RessourceManager(void)
{
}

bool RessourceManager::init(void)
{
    std::cout << "Initializing RessourceManager" << std::endl;

    if (!loadTexture("board", TEXTURE_PATH + "board.png"))
    {
        std::cerr << "Failed to load board texture" << std::endl;
        return false;
    }
    if (!loadTexture("pawn1", TEXTURE_PATH + "pawn1.png"))
    {
        std::cerr << "Failed to load pawn1 texture" << std::endl;
        return false;
    }
    if (!loadTexture("pawn2", TEXTURE_PATH + "pawn2.png"))
    {
        std::cerr << "Failed to load pawn2 texture" << std::endl;
        return false;
    }
    std::cout << "RessourceManager initialized" << std::endl;
    return true;
}

void RessourceManager::cleanup(void)
{
    std::cout << "Cleaning up RessourceManager" << std::endl;
    _textures.clear();
    std::cout << "RessourceManager cleaned up" << std::endl;
}

sf::Texture& RessourceManager::getTexture(const std::string& name)
{
    auto it = _textures.find(name);
    if (it == _textures.end())
    {
        std::cerr << "Texture " << name << " not found" << std::endl;
        throw std::runtime_error("Texture not found");
    }
    return it->second;
}

bool RessourceManager::loadTexture(const std::string& name, const std::string& path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        std::cerr << "Failed to load texture " << name << " from " << path << std::endl;
        return false;
    }
    texture.setSmooth(true);
    _textures[name] = std::move(texture);
    std::cout << "Texture " << name << " loaded" << std::endl;
    return true;
}

bool RessourceManager::hasTexture(const std::string& name) const
{
    return _textures.find(name) != _textures.end();
}
