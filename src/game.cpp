#include "game.hpp"
#include "renderer_utils.hpp"

Game::Game(SDL_Renderer* renderer)
    : renderer_(renderer)
    , audio_(bus_)
{}

void Game::clearScreen() const
{
    ::clearScreen(renderer_);
}
