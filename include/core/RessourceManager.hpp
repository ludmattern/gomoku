#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <unordered_map>

class RessourceManager
{
    public:

        RessourceManager(std::string packagePath);
        ~RessourceManager(void);

        bool init(void);
        void cleanup(void);

        sf::Texture& getTexture(const std::string& name);
        bool loadTexture(const std::string& name, const std::string& path);
        bool hasTexture(const std::string& name) const;
        
    private:

        std::unordered_map<std::string, sf::Texture> _textures;
        std::string TEXTURE_PATH;
        
};