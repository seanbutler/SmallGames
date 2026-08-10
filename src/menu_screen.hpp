#pragma once
#include "iscreen.hpp"
#include "effects.hpp"

class MenuScreen : public IScreen {
public:
    explicit MenuScreen(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

private:
    SDL_Renderer* renderer_;
    int          selected_{};
    ScreenSignal signal_{ScreenSignal::None};
    StarField    starfield_{};
    float        elapsed_{};
};
