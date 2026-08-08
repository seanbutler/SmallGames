#pragma once
#include "iscreen.hpp"
#include "spaceinvaders_game.hpp"

class SpaceInvadersScreen : public IScreen {
public:
    explicit SpaceInvadersScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    SpaceInvadersGame game_;
    ScreenSignal      signal_{ScreenSignal::None};
};
