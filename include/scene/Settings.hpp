#pragma once
#include "scene/AScene.hpp"
#include "ui/Button.hpp"


namespace gomoku::scene {

class SettingsScene : public AScene {
public:
    explicit SettingsScene(Context& ctx);
    ~SettingsScene() override = default;

    void update(sf::Time& deltaTime) override;
    void render(sf::RenderTarget& target) const override;
    bool handleInput(sf::Event& event) override;
    void onThemeChanged() override;

private:
    void applyTheme(const std::string& themeName);
    void onBackClicked();

    gomoku::ui::Button defaultBtn_;
    gomoku::ui::Button darkBtn_;
    gomoku::ui::Button halloweenBtn_;
    gomoku::ui::Button pastelBtn_;
    gomoku::ui::Button backBtn_;
};

} // namespace gomoku::scene


