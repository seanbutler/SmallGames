#pragma once
#include "game_screen.hpp"
#include "pong_game.hpp"

class PongScreen : public GameScreen {
public:
    explicit PongScreen(SDL_Renderer* renderer);
private:
    PongGame game_;
};
