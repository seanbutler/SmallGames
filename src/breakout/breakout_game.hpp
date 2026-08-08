#pragma once
#include "game.hpp"
#include "math.hpp"
#include "breakout_constants.hpp"

enum class BreakoutState { Waiting, Playing, Won, Lost };

class BreakoutGame : public Game {
public:
    explicit BreakoutGame(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;

private:
    void reset();
    void nextLevel();
    void checkWalls();
    void checkPaddle();
    void checkBricks();

    Vec2  paddle_{};
    Vec2  ball_{};
    Vec2  vel_{};
    float speed_{};

    bool bricks_[BREAKOUT_ROWS][BREAKOUT_COLS]{};
    int  bricksLeft_{};
    int  score_{};
    int  lives_{};
    int  level_{};

    BreakoutState state_{BreakoutState::Waiting};
};
