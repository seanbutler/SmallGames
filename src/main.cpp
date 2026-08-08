#include "app.hpp"
#include "constants.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cstdint>

int main(int /*argc*/, char* /*argv*/[])
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Small Games", WINDOW_W, WINDOW_H, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    App app(window, renderer);
    SDL_Event event;
    std::uint64_t lastTicks = SDL_GetTicks();

    while (app.running()) {
        while (SDL_PollEvent(&event))
            app.handleEvent(event);

        const std::uint64_t now = SDL_GetTicks();
        const float dt = std::min(0.05f, static_cast<float>(now - lastTicks) / 1000.0f);
        lastTicks = now;

        app.update(dt);
        app.draw();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
