#pragma once
#include "game.hpp"
#include "ball.hpp"
#include "paddle.hpp"
#include "pong_constants.hpp"

enum class PongState { Waiting, Playing, Paused, GameOver };

class PongGame : public Game {
public:
    explicit PongGame(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;

private:
    void checkCollisions();
    bool circleHitsRect(const SDL_FRect& rect, float* hitY) const;

    Paddle    left_;
    Paddle    right_;
    Ball      ball_;
    int       scoreLeft_{};
    int       scoreRight_{};
    PongState state_{PongState::Waiting};
};
