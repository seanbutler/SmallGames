#pragma once
#include "iscreen.hpp"
#include "effects.hpp"

class SplashScreen : public IScreen {
public:
    explicit SplashScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    SDL_Renderer* renderer_;
    float     elapsed_{};
    bool      done_{};
    StarField starfield_{};
};
