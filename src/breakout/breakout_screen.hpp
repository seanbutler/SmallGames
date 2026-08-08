#pragma once
#include "iscreen.hpp"
#include "breakout_game.hpp"

class BreakoutScreen : public IScreen {
public:
    explicit BreakoutScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    BreakoutGame game_;
    ScreenSignal signal_{ScreenSignal::None};
};
