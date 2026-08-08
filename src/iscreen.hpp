#pragma once
#include <SDL3/SDL.h>

enum class ScreenSignal { None, GoToMenu, PlayPong, PlayBreakout, PlaySpaceInvaders, PlayAsteroids, Quit };

class IScreen {
public:
    virtual ~IScreen() = default;
    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw() const = 0;
    virtual ScreenSignal signal() const { return ScreenSignal::None; }
};
