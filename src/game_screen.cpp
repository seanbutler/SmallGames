#include "game_screen.hpp"
#include <SDL3/SDL.h>

GameScreen::GameScreen(Game& game) : game_(game) {}

void GameScreen::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        signal_ = ScreenSignal::GoToMenu;
        return;
    }
    game_.handleEvent(event);
}

void GameScreen::update(float dt)
{
    if (signal_ != ScreenSignal::None) return;
    game_.update(dt);
}

void GameScreen::draw() const { game_.draw(); }

ScreenSignal GameScreen::signal() const { return signal_; }
