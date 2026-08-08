#pragma once
#include "iscreen.hpp"
#include "asteroids_game.hpp"

class AsteroidsScreen : public IScreen {
public:
    explicit AsteroidsScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    AsteroidsGame game_;
    ScreenSignal  signal_{ScreenSignal::None};
};
