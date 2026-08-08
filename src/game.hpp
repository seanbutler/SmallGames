#pragma once
#include "audio.hpp"
#include "message.hpp"
#include <SDL3/SDL.h>

class Game {
public:
    virtual ~Game() = default;
    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw() const = 0;

protected:
    explicit Game(SDL_Renderer* renderer);
    void clearScreen() const;

    SDL_Renderer* renderer_;
    MessageBus    bus_;    // must precede audio_
    AudioManager  audio_;
};
