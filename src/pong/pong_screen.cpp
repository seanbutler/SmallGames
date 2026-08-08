#include "pong_screen.hpp"

PongScreen::PongScreen(SDL_Renderer* renderer)
    : GameScreen(game_)
    , game_(renderer)
{}
