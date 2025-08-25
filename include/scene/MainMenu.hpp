#pragma once

#include "AScene.hpp"
#include "ui/Button.hpp"

class MainMenu : public AScene
{
    public:
        explicit MainMenu(Context& context);
        ~MainMenu(void);

        bool handleInput(sf::Event& event) override;
        void update(sf::Time& deltaTime) override;
        void render(sf::RenderTarget& target) const override;

    private:
        void onPlayClicked(void);
        void onSettingsClicked(void);
        void onExitClicked(void);

        Button _playButton;
        Button _settingsButton;
        Button _exitButton;
};