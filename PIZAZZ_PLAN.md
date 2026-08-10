# Pizazz Plan — SmallGames Visual Effects

## Context

The games are functionally complete but visually minimal — SDL line primitives with no feedback effects. Adding "juice" (screen shake, particles, flash, trails, floating scores) is standard game-feel polish that makes the same gameplay significantly more satisfying. This plan adds a layered effects system to the shared Game base class, then wires triggers into each game at relevant events. The splash and menu screens (which do not inherit from Game) get their own self-contained pizazz.

---

## Architecture Overview

**New file:** `src/effects.hpp` — all effect structs (header-only, no new .cpp needed for small data types).

**Modified files:**
- `src/game.hpp` / `src/game.cpp` — add effect members + helper methods
- All 4 `*_game.cpp` files — replace `clearScreen()` + `SDL_RenderPresent()` with `beginFrame()` / `presentFrame()`, add `updateEffects(dt)` call, wire triggers
- `src/CMakeLists.txt` — no change needed (effects.hpp is header-only)

**Key design:** Effect state lives in `Game` base class. `update()` (non-const) calls `updateEffects(dt)`. `draw()` (const) calls `beginFrame()` / `presentFrame()` which read effect state and call SDL. No mutable hacks needed — SDL renderer calls from const methods are already the pattern (clearScreen() already does this).

---

## Phase 1 — Infrastructure + Screen Shake + Flash

### `src/effects.hpp` (new)

```cpp
struct ShakeEffect {
    float intensity{};   // pixels of max offset
    float remaining{};   // seconds left

    void trigger(float i, float dur) { intensity = i; remaining = dur; }
    void update(float dt) { if (remaining > 0) remaining -= dt; }
    float offsetX() const { /* random offset scaled by intensity */ }
    float offsetY() const { /* same, * 0.6 for less vertical shake */ }
};

struct FlashEffect {
    Uint8 r{255}, g{255}, b{255};
    float alpha{};       // 0..1
    float remaining{};
    float duration{};

    void trigger(Uint8 R, Uint8 G, Uint8 B, float dur);
    void update(float dt);  // decays alpha linearly to 0
};
```

### `src/game.hpp` additions

```cpp
#include "effects.hpp"
// protected:
void updateEffects(float dt);
void triggerShake(float intensity, float duration);
void triggerFlash(Uint8 r, Uint8 g, Uint8 b, float duration);
void beginFrame() const;   // replaces clearScreen() — clears + sets shake viewport
void presentFrame() const; // replaces SDL_RenderPresent — draws flash + resets viewport + presents

ShakeEffect shake_;
FlashEffect  flash_;
```

### `src/game.cpp` additions

- `beginFrame()`: clears screen, then calls `SDL_SetRenderViewport` with shake offset if active
- `presentFrame()`: resets viewport to nullptr, draws flash overlay with blend mode, calls `SDL_RenderPresent`
- `updateEffects(dt)`: updates shake_ and flash_

### Per-game changes (all 4 game draw() methods)
- Replace `clearScreen();` with `beginFrame();`
- Replace `SDL_RenderPresent(renderer_);` with `presentFrame();`
- Add `updateEffects(dt);` call at start of each game's `update()`

### Trigger wiring

| Game | Event | Effect |
|------|-------|--------|
| Asteroids | ship death | `triggerShake(12, 0.4)` + `triggerFlash(255, 80, 80, 0.3)` |
| Asteroids | large asteroid destroyed | `triggerShake(6, 0.2)` |
| Breakout | life lost | `triggerShake(10, 0.35)` + `triggerFlash(255, 60, 60, 0.25)` |
| Breakout | brick hit | `triggerShake(2, 0.08)` |
| Pong | goal scored | `triggerShake(8, 0.3)` + `triggerFlash(255, 255, 255, 0.15)` |
| Space Invaders | player hit | `triggerShake(10, 0.4)` + `triggerFlash(255, 60, 60, 0.3)` |
| Space Invaders | invader killed | `triggerShake(2, 0.07)` |

---

## Phase 2 — Particle System

### Add to `src/effects.hpp`

```cpp
struct Particle {
    float x, y, vx, vy;
    float life, maxLife;
    Uint8 r, g, b;
    bool  active{};
};

struct ParticleSystem {
    static constexpr int MAX = 256;
    Particle pool[MAX]{};

    void emit(float x, float y, int count, float speed, Uint8 r, Uint8 g, Uint8 b);
    void update(float dt);
    void draw(SDL_Renderer* renderer) const;
};
```

Particle velocity: random direction, random speed in [speed*0.5, speed*1.5]. Life: 0.4–0.8s random. Size: 2px square.

### `src/game.hpp`
- Add `ParticleSystem particles_` to protected section
- `updateEffects(dt)` also calls `particles_.update(dt)`
- `presentFrame()` calls `particles_.draw(renderer_)` before flash overlay (flash goes on top)

### Trigger wiring

| Game | Event | Emit |
|------|-------|------|
| Asteroids | large asteroid destroyed | `emit(pos, 12, 90, white)` |
| Asteroids | medium asteroid destroyed | `emit(pos, 8, 120, white)` |
| Asteroids | small asteroid destroyed | `emit(pos, 5, 150, white)` |
| Asteroids | ship death | `emit(shipPos, 20, 80, orange)` |
| Breakout | brick hit | `emit(brickCenter, 6, 100, brickColor)` |
| Pong | goal scored | `emit(ballPos, 10, 80, white)` |
| Space Invaders | invader killed | `emit(invaderPos, 8, 70, green)` |

---

## Phase 3 — Motion Trails

Simple ring buffer per game for ship/ball position history. Each game adds its own — no base class change needed.

### Asteroids ship trail
- Add `Vec2 shipTrail_[8]{}` and `int trailHead_{}` to `AsteroidsGame`
- Push `shipPos_` into ring buffer each frame in `updateShip()`
- Draw as 1px dots stepping from white to dark grey

### Pong ball trail
- Add `Vec2 ballTrail_[6]{}` + `int ballTrailHead_{}` to `PongGame`
- Updated each frame when Playing
- Drawn as small 2px squares in grey tones

### Breakout ball trail
- Same pattern — 6-position ring buffer for `ball_` Vec2

---

## Phase 4 — Floating Score Text

### Add to `src/effects.hpp`

```cpp
struct FloatingText {
    char  text[8]{};
    float x, y, vy{-60.0f};
    float life{0.9f}, maxLife{0.9f};
    float scale{4.0f};
    bool  active{};
};

struct FloatingTextSystem {
    static constexpr int MAX = 16;
    FloatingText pool[MAX]{};
    void spawn(const char* text, float x, float y);
    void update(float dt);
    void draw(SDL_Renderer* renderer) const;  // uses drawText from renderer_utils
};
```

### `src/game.hpp`
- Add `FloatingTextSystem floatingTexts_` to protected section
- `updateEffects(dt)` updates it
- `presentFrame()` calls `floatingTexts_.draw(renderer_)` before flash

### Trigger wiring
- Asteroids: `floatingTexts_.spawn("+100", ast.pos.x, ast.pos.y)` in bullet-vs-asteroid
- Breakout: `floatingTexts_.spawn("+7", brickCenter.x, brickCenter.y)` in checkBricks
- Space Invaders: at invader kill position

---

## Phase 5 — Starfield Background

A twinkling starfield drawn behind all game content and on the splash/menu screens.

### Add to `src/effects.hpp`

```cpp
struct StarField {
    struct Star { float x, y, brightness, phase, speed; };
    static constexpr int COUNT = 120;
    Star stars[COUNT]{};
    float elapsed{};

    StarField();               // generate random positions + phases
    void update(float dt);     // advance elapsed for twinkle
    void draw(SDL_Renderer* r) const;  // single pixels, brightness via grey shade
};
```

- Stars distributed randomly across the window at construction (using `randInt`)
- Three rough "depth" layers: 40 dim slow-twinkling far stars, 50 mid stars, 30 bright near stars
- Brightness = `base + std::sinf(elapsed * speed + phase) * variance` — each star twinkles independently
- Drawn as single pixels (`SDL_RenderPoint`) in grey shades (30–200 range to stay subtle)
- No parallax or scrolling for now — keep it simple and static in position

### Wiring

**In `Game` base class** — add `StarField starfield_` to protected section. `updateEffects(dt)` calls `starfield_.update(dt)`. `beginFrame()` calls `starfield_.draw(renderer_)` after `SDL_RenderClear` and before returning (so game content draws on top).

All four games get it automatically. Works especially well for Asteroids and Space Invaders. Subtle enough for Pong/Breakout.

**In `SplashScreen` and `MenuScreen`** — each creates its own local `StarField` member. Drawn in `draw()` after `clearScreen(renderer_)` and before the text layers.

Changes: `splash_screen.hpp` adds `StarField starfield_{}`, `menu_screen.hpp` adds `StarField starfield_{}`. Both `.cpp` files call `starfield_.update(dt)` in `update()` and `starfield_.draw(renderer_)` in `draw()`.

---

## Phase 7 — Splash & Menu Pizazz

`SplashScreen` and `MenuScreen` inherit directly from `IScreen` (not `Game`), so they get none of the Game effects infrastructure. They each own their own `SDL_Renderer*` and call `SDL_RenderPresent` themselves.

### Shared helper: `drawFadeOverlay(renderer, alpha)` in renderer_utils

```cpp
// alpha 0..1, draws a black rect over the entire screen at given opacity
void drawFadeOverlay(SDL_Renderer* r, float alpha);
```

Uses `SDL_BLENDMODE_BLEND`. Added to `renderer_utils.hpp/.cpp`.

---

### SplashScreen

`elapsed_` already exists (tracks 0..2s). Use it for:

1. **Fade in** (0.0–0.4s): Call `drawFadeOverlay(renderer_, 1.0f - elapsed_ / 0.4f)` before `SDL_RenderPresent`. After 0.4s overlay is fully transparent.
2. **Blinking subtitle**: "PRESS ANY KEY" shown only when `static_cast<int>(elapsed_ * 2) % 2 == 0` (blinks at 2 Hz after the fade-in is done).
3. **Fade out** (last 0.3s before done_): If `elapsed_ >= 1.7f`, draw `drawFadeOverlay` with alpha ramping back to 1.0.

No new members needed — `elapsed_` drives everything.

---

### MenuScreen

Currently `update(float /*dt*/)` does nothing. Add `elapsed_` member and accumulate dt.

1. **Animated cursor**: Instead of drawing a static `>`, draw it offset by `std::sinf(elapsed_ * 6.0f) * 4.0f` pixels horizontally — gives a gentle left/right pulse.
2. **Selected item highlight**: Draw selected item in a warm yellow (`255, 220, 80`) instead of white. Non-selected items stay white.
3. **Fade in on entry**: Draw `drawFadeOverlay` with alpha = `std::max(0.0f, 1.0f - elapsed_ / 0.3f)` — fades from black to transparent over 0.3s on first entry.

Changes to `menu_screen.hpp`: add `float elapsed_{}`.
Changes to `menu_screen.cpp`: `update(float dt)` accumulates `elapsed_ += dt`; draw() uses elapsed for cursor pulse and fade-in.

---

### Screen transitions (App level)

`App::transition()` is instant today — old screen destroyed, new one created. To add a brief black flash between screens:

Add `float transitionFlash_{}` to `App`. When `transition()` fires, set `transitionFlash_ = 0.25f`. In `App::draw()`, after `screen_->draw()`, if `transitionFlash_ > 0`:

```
// Problem: screen_->draw() already called SDL_RenderPresent().
// Can't composite on top of a presented frame.
```

**Constraint**: Every screen ends its `draw()` with `SDL_RenderPresent()`, so App has no hook to draw on top. Options:

- **Option A (recommended)**: Move `SDL_RenderPresent` out of all screens and into `App::draw()`. This is the cleanest long-term change but touches every draw() method.
- **Option B (deferred)**: Skip app-level transitions for now. Each screen handles its own fade-in (phases above cover this for splash and menu; games get it via `beginFrame`/`presentFrame` in Phase 1).

**Decision**: Go with Option B for now — each screen fades itself in. True cross-screen dissolves can be added later by moving `SDL_RenderPresent` to App (a separate refactor).

---

## Files Changed Summary

| File | Change |
|------|--------|
| `src/effects.hpp` | **NEW** — all effect structs |
| `src/game.hpp` | Add effect members + method declarations |
| `src/game.cpp` | Implement updateEffects, triggerShake, triggerFlash, beginFrame, presentFrame |
| `src/asteroids/asteroids_game.hpp` | Add trail ring buffer |
| `src/asteroids/asteroids_game.cpp` | Wire all triggers + trail update/draw |
| `src/breakout/breakout_game.hpp` | Add trail ring buffer |
| `src/breakout/breakout_game.cpp` | Wire all triggers + trail update/draw |
| `src/pong/pong_game.hpp` | Add trail ring buffer |
| `src/pong/pong_game.cpp` | Wire all triggers + trail update/draw |
| `src/spaceinvaders/spaceinvaders_game.cpp` | Wire shake/flash/particle triggers |
| `src/renderer_utils.hpp` / `.cpp` | Add `drawFadeOverlay(renderer, alpha)` |
| `src/splash_screen.hpp` | Add `StarField starfield_` member |
| `src/splash_screen.cpp` | Starfield draw, fade in, blinking subtitle, fade out |
| `src/menu_screen.hpp` | Add `elapsed_`, `StarField starfield_` members |
| `src/menu_screen.cpp` | Starfield draw, accumulate dt, pulsing cursor, yellow highlight, fade in |

---

## Verification

1. Build: `cmake --build build` — should compile cleanly with no new warnings
2. Launch each game and verify:
   - **Asteroids**: ship flashes red + shakes on death; asteroids burst into particles on hit; ship leaves faint trail; floating "+20"/"+50"/"+100" on kills
   - **Breakout**: brick colored particles on hit; screen shakes; life-loss flashes red; ball trails
   - **Pong**: goal scored flashes + shakes; ball trails
   - **Space Invaders**: invader particles; flash + shake on player hit
3. Verify no regressions: game logic unchanged, escape/menu still works, score still counts correctly
4. Verify shake doesn't leave permanent viewport offset (`presentFrame` always resets with `SDL_SetRenderViewport(renderer_, nullptr)`)
