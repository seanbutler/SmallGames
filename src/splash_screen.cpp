#include "splash_screen.hpp"
#include "constants.hpp"
#include "renderer_utils.hpp"

SplashScreen::SplashScreen(SDL_Renderer* renderer)
    : renderer_(renderer)
{}

void SplashScreen::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        done_ = true;
}

void SplashScreen::update(float dt)
{
    elapsed_ += dt;
    if (elapsed_ >= 2.0f) done_ = true;
}

void SplashScreen::draw() const
{
    clearScreen(renderer_);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    constexpr float titleScale = 14.0f;
    const char* title = "SMALL GAMES";
    const float tw = textWidth(title, titleScale);
    drawText(renderer_, title,
             WINDOW_W / 2.0f - tw / 2.0f,
             WINDOW_H / 2.0f - 5.0f * titleScale,
             titleScale);

    constexpr float subScale = 6.0f;
    const char* sub = "PRESS ANY KEY";
    const float sw = textWidth(sub, subScale);
    drawText(renderer_, sub,
             WINDOW_W / 2.0f - sw / 2.0f,
             WINDOW_H / 2.0f + 60.0f,
             subScale);

    SDL_RenderPresent(renderer_);
}

ScreenSignal SplashScreen::signal() const
{
    return done_ ? ScreenSignal::GoToMenu : ScreenSignal::None;
}
