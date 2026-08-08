#include "game.hpp"

Game::Game(SDL_Renderer* renderer)
    : renderer_(renderer)
    , audio_(bus_)
{}

void Game::clearScreen() const
{
    SDL_SetRenderDrawColor(renderer_, 20, 20, 28, 255);
    SDL_RenderClear(renderer_);
}
