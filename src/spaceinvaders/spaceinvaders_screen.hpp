#pragma once
#include "game_screen.hpp"
#include "spaceinvaders_game.hpp"

class SpaceInvadersScreen : public GameScreen {
public:
    explicit SpaceInvadersScreen(SDL_Renderer* renderer);
private:
    SpaceInvadersGame game_;
};
