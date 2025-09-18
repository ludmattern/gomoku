#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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
    bool loadTextureIfExists(const std::string& name, const std::string& path);
    bool hasTexture(const std::string& name) const;

    // Audio (SFX): load and access sound buffers
    bool loadSound(const std::string& name, const std::string& path);
    bool loadSoundOptional(const std::string& name, const std::string& path);
    bool hasSound(const std::string& name) const;
    const sf::SoundBuffer* getSound(const std::string& name) const;

    // Reload all SFX for a given theme under canonical keys (with fallback to default)
    bool setAudioPackage(const std::string& theme);

    // Themes: switch and reload textures for selected theme
    bool setTexturePackage(const std::string& theme);
    const std::string& currentTexturePackage() const { return texturePath_; }

private:
    std::unordered_map<std::string, sf::Texture> textures_;
    std::string texturePath_;
    std::unordered_map<std::string, sf::SoundBuffer> sounds_;
};

} // namespace gomoku::gui
