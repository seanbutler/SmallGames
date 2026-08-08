#pragma once
#include "game_screen.hpp"
#include "breakout_game.hpp"

class BreakoutScreen : public GameScreen {
public:
    explicit BreakoutScreen(SDL_Renderer* renderer);
private:
    BreakoutGame game_;
};
