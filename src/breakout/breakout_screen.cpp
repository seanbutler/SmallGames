#include "breakout_screen.hpp"

BreakoutScreen::BreakoutScreen(SDL_Renderer* renderer)
    : game_(renderer)
{}

void BreakoutScreen::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        signal_ = ScreenSignal::GoToMenu;
        return;
    }
    game_.handleEvent(event);
}

void BreakoutScreen::update(float dt)
{
    if (signal_ != ScreenSignal::None) return;
    game_.update(dt);
}

void BreakoutScreen::draw() const
{
    game_.draw();
}

ScreenSignal BreakoutScreen::signal() const { return signal_; }
