#pragma once
#include "scene/AScene.hpp"
#include "ui/Button.hpp"

class GameSelectScene : public AScene {
public:
    explicit GameSelectScene(Context& ctx);
    ~GameSelectScene() override = default;

    void update(sf::Time& deltaTime) override;
    void render(sf::RenderTarget& target) const override;
    bool handleInput(sf::Event& event) override;

private:
    void onPlayerVsPlayerClicked(void);
    void onPlayerVsBotClicked(void);
    void onBackClicked(void);

    Button _playerVsPlayerButton;
    Button _playerVsBotButton;
    Button _backButton;
};