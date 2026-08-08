#include "menu_screen.hpp"
#include "constants.hpp"
#include "renderer_utils.hpp"
#include <algorithm>

namespace {

struct MenuItem {
    const char*  label;
    ScreenSignal action;
};

constexpr MenuItem ITEMS[] = {
    {"PONG",      ScreenSignal::PlayPong},
    {"BREAKOUT",  ScreenSignal::PlayBreakout},
    {"INVADERS",  ScreenSignal::PlaySpaceInvaders},
    {"ASTEROIDS", ScreenSignal::PlayAsteroids},
};

constexpr int ITEM_COUNT = static_cast<int>(sizeof(ITEMS) / sizeof(ITEMS[0]));

} // namespace

MenuScreen::MenuScreen(SDL_Renderer* renderer)
    : renderer_(renderer)
{}

void MenuScreen::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    switch (event.key.key) {
        case SDLK_UP:
            selected_ = (selected_ - 1 + ITEM_COUNT) % ITEM_COUNT;
            break;
        case SDLK_DOWN:
            selected_ = (selected_ + 1) % ITEM_COUNT;
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            signal_ = ITEMS[selected_].action;
            break;
        case SDLK_ESCAPE:
            signal_ = ScreenSignal::Quit;
            break;
        default:
            break;
    }
}

void MenuScreen::update(float /*dt*/) {}

void MenuScreen::draw() const
{
    clearScreen(renderer_);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    constexpr float titleScale = 12.0f;
    const char* title = "SMALL GAMES";
    const float tw = textWidth(title, titleScale);
    drawText(renderer_, title, WINDOW_W / 2.0f - tw / 2.0f, 140.0f, titleScale);

    constexpr float itemScale   = 8.0f;
    constexpr float itemSpacing = 70.0f;
    const float listTop = WINDOW_H / 2.0f - (static_cast<float>(ITEM_COUNT) * itemSpacing) / 2.0f;

    // Fixed cursor column: left of the widest label so it never jumps between items
    float maxLabelW = 0.0f;
    for (int i = 0; i < ITEM_COUNT; ++i)
        maxLabelW = std::max(maxLabelW, textWidth(ITEMS[i].label, itemScale));
    const float cursorX = WINDOW_W / 2.0f - maxLabelW / 2.0f - 5.0f * itemScale;

    for (int i = 0; i < ITEM_COUNT; ++i) {
        const char* label = ITEMS[i].label;
        const float lw = textWidth(label, itemScale);
        const float y  = listTop + static_cast<float>(i) * itemSpacing;

        if (i == selected_)
            drawText(renderer_, ">", cursorX, y, itemScale);

        drawText(renderer_, label, WINDOW_W / 2.0f - lw / 2.0f, y, itemScale);
    }

    constexpr float footerScale = 5.0f;
    const char* footer = "ENTER TO SELECT  ESC TO QUIT";
    const float fw = textWidth(footer, footerScale);
    drawText(renderer_, footer, WINDOW_W / 2.0f - fw / 2.0f, WINDOW_H - 80.0f, footerScale);

    SDL_RenderPresent(renderer_);
}

ScreenSignal MenuScreen::signal() const { return signal_; }
