#include "app.hpp"
#include "splash_screen.hpp"
#include "menu_screen.hpp"
#include "pong_screen.hpp"
#include "breakout_screen.hpp"
#include "spaceinvaders_screen.hpp"
#include "asteroids_screen.hpp"

App::App(SDL_Window* window, SDL_Renderer* renderer)
    : window_(window)
    , renderer_(renderer)
    , screen_(std::make_unique<SplashScreen>(renderer))
{}

void App::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
        return;
    }
    if (screen_) screen_->handleEvent(event);
}

void App::update(float dt)
{
    if (!screen_) return;
    screen_->update(dt);

    const ScreenSignal sig = screen_->signal();
    if (sig != ScreenSignal::None)
        transition(sig);
}

void App::draw() const
{
    if (screen_) screen_->draw();
}

void App::transition(ScreenSignal signal)
{
    switch (signal) {
        case ScreenSignal::GoToMenu:
            screen_ = std::make_unique<MenuScreen>(renderer_);
            break;
        case ScreenSignal::PlayPong:
            screen_ = std::make_unique<PongScreen>(renderer_);
            break;
        case ScreenSignal::PlayBreakout:
            screen_ = std::make_unique<BreakoutScreen>(renderer_);
            break;
        case ScreenSignal::PlaySpaceInvaders:
            screen_ = std::make_unique<SpaceInvadersScreen>(renderer_);
            break;
        case ScreenSignal::PlayAsteroids:
            screen_ = std::make_unique<AsteroidsScreen>(renderer_);
            break;
        case ScreenSignal::Quit:
            running_ = false;
            break;
        default:
            break;
    }
}
