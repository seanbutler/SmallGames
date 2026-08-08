#include "pong_game.hpp"
#include "pong_constants.hpp"
#include "renderer_utils.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

PongGame::PongGame(SDL_Renderer* renderer)
    : Game(renderer)
    , left_(Side::Left)
    , right_(Side::Right)
    , ball_(bus_)
{
    ball_.serve(true);
}

void PongGame::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
        return;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) return;

    switch (event.key.key) {
        case SDLK_ESCAPE:
            running_ = false;
            break;
        case SDLK_P:
            if (state_ == PongState::Playing)      state_ = PongState::Paused;
            else if (state_ == PongState::Paused)  state_ = PongState::Playing;
            break;
        case SDLK_SPACE:
            if (state_ == PongState::Waiting) {
                state_ = PongState::Playing;
                ball_.serve(ball_.vel.x >= 0.0f);
            } else if (state_ == PongState::GameOver) {
                scoreLeft_  = 0;
                scoreRight_ = 0;
                state_      = PongState::Waiting;
                ball_.serve(true);
            }
            break;
        default:
            break;
    }
}

void PongGame::update(float dt)
{
    if (state_ != PongState::Playing) return;

    left_.update(dt);
    right_.update(dt);
    ball_.update(dt);
    checkCollisions();

    if (ball_.pos.x < -50.0f) {
        ++scoreRight_;
        bus_.publish(Event::Score);
        state_ = (scoreRight_ >= WIN_SCORE) ? PongState::GameOver : PongState::Waiting;
        ball_.serve(true);
    } else if (ball_.pos.x > WINDOW_W + 50.0f) {
        ++scoreLeft_;
        bus_.publish(Event::Score);
        state_ = (scoreLeft_ >= WIN_SCORE) ? PongState::GameOver : PongState::Waiting;
        ball_.serve(false);
    }
}

void PongGame::draw() const
{
    clearScreen();
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    drawCenterLine(renderer_);
    left_.draw(renderer_);
    right_.draw(renderer_);
    ball_.draw(renderer_);

    drawNumber(renderer_, scoreLeft_,  WINDOW_W / 2.0f - 130.0f, 24.0f, 14.0f);
    drawNumber(renderer_, scoreRight_, WINDOW_W / 2.0f +  70.0f, 24.0f, 14.0f);

    if (state_ == PongState::GameOver) {
        const char* winner = (scoreLeft_ >= WIN_SCORE) ? "PLAYER ONE WINS" : "PLAYER TWO WINS";
        const char* opts   = "SPACE REPLAY  ESC MENU";
        const float w1 = textWidth(winner, 7.0f);
        const float w2 = textWidth(opts,   5.0f);
        drawText(renderer_, winner, WINDOW_W / 2.0f - w1 / 2.0f, WINDOW_H / 2.0f + 60.0f, 7.0f);
        drawText(renderer_, opts,   WINDOW_W / 2.0f - w2 / 2.0f, WINDOW_H / 2.0f + 120.0f, 5.0f);
    } else if (state_ != PongState::Playing) {
        const char* msg = (state_ == PongState::Paused) ? "PAUSED  PRESS P"
                                                        : "PRESS SPACE TO SERVE";
        const float msgW = textWidth(msg, 7.0f);
        drawText(renderer_, msg, WINDOW_W / 2.0f - msgW / 2.0f, WINDOW_H / 2.0f + 90.0f, 7.0f);
    }

    SDL_RenderPresent(renderer_);
}

void PongGame::checkCollisions()
{
    float hitY{};

    if (ball_.vel.x < 0.0f && circleHitsRect(left_.rect(), &hitY)) {
        const float rel   = std::clamp(hitY, -1.0f, 1.0f);
        const float angle = rel * 60.0f * std::numbers::pi_v<float> / 180.0f;
        ball_.vel   = {std::cosf(angle), std::sinf(angle)};
        ball_.speed = std::min(ball_.speed + BALL_SPEED_STEP, BALL_SPEED_MAX);
        ball_.pos.x = PADDLE_INSET + PADDLE_W + BALL_R;
        ball_.pos.y = std::clamp(ball_.pos.y, BALL_R, WINDOW_H - BALL_R);
        bus_.publish(Event::PaddleHit);
    } else if (ball_.vel.x > 0.0f && circleHitsRect(right_.rect(), &hitY)) {
        const float rel   = std::clamp(hitY, -1.0f, 1.0f);
        const float angle = rel * 60.0f * std::numbers::pi_v<float> / 180.0f;
        ball_.vel   = {-std::cosf(angle), std::sinf(angle)};
        ball_.speed = std::min(ball_.speed + BALL_SPEED_STEP, BALL_SPEED_MAX);
        ball_.pos.x = WINDOW_W - PADDLE_INSET - PADDLE_W - BALL_R;
        ball_.pos.y = std::clamp(ball_.pos.y, BALL_R, WINDOW_H - BALL_R);
        bus_.publish(Event::PaddleHit);
    }
}

bool PongGame::circleHitsRect(const SDL_FRect& r, float* hitY) const
{
    const float nx = std::clamp(ball_.pos.x, r.x, r.x + r.w);
    const float ny = std::clamp(ball_.pos.y, r.y, r.y + r.h);
    const float dx = ball_.pos.x - nx;
    const float dy = ball_.pos.y - ny;
    if (dx * dx + dy * dy > BALL_R * BALL_R) return false;

    const float centerY = r.y + r.h / 2.0f;
    *hitY = (ball_.pos.y - centerY) / (r.h / 2.0f);
    return true;
}
