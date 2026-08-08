#pragma once
#include "iscreen.hpp"
#include "pong_game.hpp"

class PongScreen : public IScreen {
public:
    explicit PongScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    PongGame     game_;
    ScreenSignal signal_{ScreenSignal::None};
};
