#include "asteroids_screen.hpp"

AsteroidsScreen::AsteroidsScreen(SDL_Renderer* renderer)
    : GameScreen(game_)
    , game_(renderer)
{}
