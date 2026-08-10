#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include "constants.hpp"
#include "rng.hpp"

struct StarField {
    struct Star {
        float x, y;
        float vx, vy;
        float baseBright;
        float twinkleAmp;
        float twinkleSpeed;
        float phase;
        bool  large;   // 2x2 px instead of 1x1
    };

    static constexpr int COUNT = 120;
    Star  stars[COUNT]{};
    float elapsed{};

    StarField()
    {
        const float angle = static_cast<float>(randInt(0, 999)) * (2.0f * 3.14159f / 1000.0f);
        const float DX    = std::cos(angle);
        const float DY    = std::sin(angle);
        for (int i = 0; i < COUNT; ++i) {
            Star& s = stars[i];
            s.x     = static_cast<float>(randInt(0, WINDOW_W - 1));
            s.y     = static_cast<float>(randInt(0, WINDOW_H - 1));
            s.phase = static_cast<float>(randInt(0, 628)) / 100.0f;

                // shared drift direction: same for all stars, random each run
            if (i < 50) {
                // far — dim, slow, 1px
                const float spd = 6.0f + static_cast<float>(randInt(0, 60)) / 10.0f;
                s.vx            = DX * spd;
                s.vy            = DY * spd;
                s.baseBright    = 0.12f + static_cast<float>(randInt(0, 20)) / 100.0f;
                s.twinkleAmp    = 0.04f;
                s.twinkleSpeed  = 0.4f + static_cast<float>(randInt(0, 60)) / 100.0f;
                s.large         = false;
            } else if (i < 95) {
                // mid
                const float spd = 18.0f + static_cast<float>(randInt(0, 120)) / 10.0f;
                s.vx            = DX * spd;
                s.vy            = DY * spd;
                s.baseBright    = 0.35f + static_cast<float>(randInt(0, 30)) / 100.0f;
                s.twinkleAmp    = 0.10f;
                s.twinkleSpeed  = 1.0f + static_cast<float>(randInt(0, 100)) / 100.0f;
                s.large         = false;
            } else {
                // near — bright, fast, 2px
                const float spd = 38.0f + static_cast<float>(randInt(0, 120)) / 10.0f;
                s.vx            = DX * spd;
                s.vy            = DY * spd;
                s.baseBright    = 0.65f + static_cast<float>(randInt(0, 25)) / 100.0f;
                s.twinkleAmp    = 0.20f;
                s.twinkleSpeed  = 2.0f + static_cast<float>(randInt(0, 200)) / 100.0f;
                s.large         = true;
            }
        }
    }

    void update(float dt)
    {
        elapsed += dt;
        for (auto& s : stars) {
            s.x += s.vx * dt;
            s.y += s.vy * dt;
            if (s.x < 0.0f)          s.x += static_cast<float>(WINDOW_W);
            if (s.x >= WINDOW_W)     s.x -= static_cast<float>(WINDOW_W);
            if (s.y < 0.0f)          s.y += static_cast<float>(WINDOW_H);
            if (s.y >= WINDOW_H)     s.y -= static_cast<float>(WINDOW_H);
        }
    }

    void draw(SDL_Renderer* r) const
    {
        for (const auto& s : stars) {
            const float b    = std::clamp(s.baseBright + std::sinf(elapsed * s.twinkleSpeed + s.phase) * s.twinkleAmp, 0.0f, 1.0f);
            const Uint8 grey = static_cast<Uint8>(b * 255.0f);
            SDL_SetRenderDrawColor(r, grey, grey, grey, 255);
            if (s.large) {
                const SDL_FRect dot{s.x, s.y, 2.0f, 2.0f};
                SDL_RenderFillRect(r, &dot);
            } else {
                SDL_RenderPoint(r, s.x, s.y);
            }
        }
    }
};
