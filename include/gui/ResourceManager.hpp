#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace gomoku::gui {

class ResourceManager {
public:
    explicit ResourceManager(std::string packagePath);
    ~ResourceManager();

    bool init();
    void cleanup();

    sf::Texture& getTexture(const std::string& name);
    bool loadTexture(const std::string& name, const std::string& path);
    bool hasTexture(const std::string& name) const;

private:
    std::unordered_map<std::string, sf::Texture> textures_;
    std::string texturePath_;
};

} // namespace gomoku::gui
