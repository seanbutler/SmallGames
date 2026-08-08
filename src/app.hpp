#pragma once
#include "iscreen.hpp"
#include <SDL3/SDL.h>
#include <memory>

class App {
public:
    App(SDL_Window* window, SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event);
    void update(float dt);
    void draw() const;
    bool running() const { return running_; }

private:
    void transition(ScreenSignal signal);

    SDL_Window*              window_;
    SDL_Renderer*            renderer_;
    std::unique_ptr<IScreen> screen_;
    bool                     running_{true};
};
