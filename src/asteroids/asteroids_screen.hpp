#pragma once
#include "game_screen.hpp"
#include "asteroids_game.hpp"

class AsteroidsScreen : public GameScreen {
public:
    explicit AsteroidsScreen(SDL_Renderer* renderer);
private:
    AsteroidsGame game_;
};
