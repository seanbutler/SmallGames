#include "pong_screen.hpp"

PongScreen::PongScreen(SDL_Renderer* renderer)
    : game_(renderer)
{}

void PongScreen::handleEvent(const SDL_Event& event)
{
    // Intercept Escape: go back to menu instead of quitting the process
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        signal_ = ScreenSignal::GoToMenu;
        return;
    }
    game_.handleEvent(event);
}

void PongScreen::update(float dt)
{
    if (signal_ != ScreenSignal::None) return;
    game_.update(dt);
}

void PongScreen::draw() const
{
    game_.draw();
}

ScreenSignal PongScreen::signal() const { return signal_; }
