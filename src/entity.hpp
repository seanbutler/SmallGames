#pragma once
#include "math.hpp"
#include <SDL3/SDL.h>

class Entity {
public:
    Vec2 pos{};

    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    virtual void draw(SDL_Renderer* renderer) const = 0;
};
