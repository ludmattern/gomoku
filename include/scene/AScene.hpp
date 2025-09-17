#pragma once
#include "scene/Context.hpp"
#include <SFML/Graphics.hpp>

namespace gomoku::scene {

class AScene {
public:
    explicit AScene(Context& context);
    virtual ~AScene() = default;

    virtual void onEnter() { }
    virtual void onExit() { }
    virtual void onThemeChanged() { }

    virtual bool handleInput(sf::Event& event) = 0;
    virtual void update(sf::Time& deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

protected:
    void playSfx(const char* name, float volume = 100.f) const;
    void playMusic(const char* path, bool loop = true, float volume = 60.f) const;
    Context& context_;
};

} // namespace gomoku::scene
