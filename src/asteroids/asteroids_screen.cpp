#include "asteroids_screen.hpp"

AsteroidsScreen::AsteroidsScreen(SDL_Renderer* renderer)
    : game_(renderer)
{}

void AsteroidsScreen::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        signal_ = ScreenSignal::GoToMenu;
        return;
    }
    game_.handleEvent(event);
}

void AsteroidsScreen::update(float dt)
{
    if (signal_ != ScreenSignal::None) return;
    game_.update(dt);
}

void AsteroidsScreen::draw() const
{
    game_.draw();
}

ScreenSignal AsteroidsScreen::signal() const { return signal_; }
