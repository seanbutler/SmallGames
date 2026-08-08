#include "paddle.hpp"
#include "pong_constants.hpp"
#include <algorithm>

Paddle::Paddle(Side side)
    : side_(side)
{
    pos.y = WINDOW_H / 2.0f;
    if (side == Side::Left) {
        pos.x  = PADDLE_INSET;
        keyUp_   = SDL_SCANCODE_W;
        keyDown_ = SDL_SCANCODE_S;
    } else {
        pos.x  = WINDOW_W - PADDLE_INSET - PADDLE_W;
        keyUp_   = SDL_SCANCODE_UP;
        keyDown_ = SDL_SCANCODE_DOWN;
    }
}

void Paddle::update(float dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    if (keys[keyUp_])   pos.y -= PADDLE_SPEED * dt;
    if (keys[keyDown_]) pos.y += PADDLE_SPEED * dt;
    pos.y = std::clamp(pos.y, PADDLE_H / 2.0f, WINDOW_H - PADDLE_H / 2.0f);
}

void Paddle::draw(SDL_Renderer* renderer) const
{
    SDL_FRect r = rect();
    SDL_RenderFillRect(renderer, &r);
}

SDL_FRect Paddle::rect() const
{
    return {pos.x, pos.y - PADDLE_H / 2.0f, PADDLE_W, PADDLE_H};
}
