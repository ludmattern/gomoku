#pragma once
#include "scene/AScene.hpp"
#include "ui/Button.hpp"

namespace gomoku::scene {

class GameSelectScene : public AScene {
public:
    explicit GameSelectScene(Context& ctx);
    ~GameSelectScene() override = default;

    void update(sf::Time& deltaTime) override;
    void render(sf::RenderTarget& target) const override;
    bool handleInput(sf::Event& event) override;

private:
    void onPlayerVsPlayerClicked();
    void onPlayerVsBotClicked();
    void onBackClicked();

    gomoku::ui::Button playerVsPlayerButton_;
    gomoku::ui::Button playerVsBotButton_;
    gomoku::ui::Button backButton_;
};

} // namespace gomoku::scene
