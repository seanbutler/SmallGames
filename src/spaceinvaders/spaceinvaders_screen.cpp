#include "spaceinvaders_screen.hpp"

SpaceInvadersScreen::SpaceInvadersScreen(SDL_Renderer* renderer)
    : game_(renderer)
{}

void SpaceInvadersScreen::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        signal_ = ScreenSignal::GoToMenu;
        return;
    }
    game_.handleEvent(event);
}

void SpaceInvadersScreen::update(float dt)
{
    if (signal_ != ScreenSignal::None) return;
    game_.update(dt);
}

void SpaceInvadersScreen::draw() const
{
    game_.draw();
}

ScreenSignal SpaceInvadersScreen::signal() const { return signal_; }
