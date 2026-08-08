#pragma once
#include "entity.hpp"
#include <SDL3/SDL.h>

enum class Side { Left, Right };

class Paddle : public Entity {
public:
    explicit Paddle(Side side);

    void update(float dt) override;
    void draw(SDL_Renderer* renderer) const override;

    SDL_FRect rect() const;

private:
    Side side_;
    SDL_Scancode keyUp_;
    SDL_Scancode keyDown_;
};
