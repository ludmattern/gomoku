#pragma once

#include "scene/AScene.hpp"
#include "ui/Button.hpp"

namespace gomoku::scene {

class MainMenu : public AScene {
public:
    explicit MainMenu(Context& context);
    ~MainMenu();

    bool handleInput(sf::Event& event) override;
    void update(sf::Time& deltaTime) override;
    void render(sf::RenderTarget& target) const override;

private:
    void onPlayClicked();
    void onSettingsClicked();
    void onExitClicked();

    gomoku::ui::Button playButton_;
    gomoku::ui::Button settingsButton_;
    gomoku::ui::Button exitButton_;
};

} // namespace gomoku::scene
