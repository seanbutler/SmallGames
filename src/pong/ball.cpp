#include "ball.hpp"
#include "pong_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <SDL3/SDL.h>

Ball::Ball(MessageBus& bus)
    : bus_(bus)
{
    pos = {WINDOW_W / 2.0f, WINDOW_H / 2.0f};
}

void Ball::update(float dt)
{
    pos.x += vel.x * speed * dt;
    pos.y += vel.y * speed * dt;

    if (pos.y < BALL_R || pos.y > WINDOW_H - BALL_R) {
        pos.y = std::clamp(pos.y, BALL_R, WINDOW_H - BALL_R);
        vel.y = -vel.y;
        bus_.publish(Event::WallBounce);
    }
}

void Ball::draw(SDL_Renderer* renderer) const
{
    SDL_FRect r{pos.x - BALL_R, pos.y - BALL_R, BALL_R * 2.0f, BALL_R * 2.0f};
    SDL_RenderFillRect(renderer, &r);
}

void Ball::serve(bool moveRight)
{
    pos   = {WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    speed = BALL_SPEED_BASE;

    const float angle = static_cast<float>(std::rand() % 46 - 22) * std::numbers::pi_v<float> / 180.0f;
    const float dir   = moveRight ? 1.0f : -1.0f;
    vel = {std::cosf(angle) * dir, std::sinf(angle)};
}
