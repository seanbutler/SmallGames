#include "breakout_screen.hpp"

BreakoutScreen::BreakoutScreen(SDL_Renderer* renderer)
    : GameScreen(game_)
    , game_(renderer)
{}
