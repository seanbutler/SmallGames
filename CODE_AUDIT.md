# SmallGames Code Audit

## Duplication / DRY

1. **[DONE]** ~~**All four `XxxScreen` classes are identical boilerplate**~~ — Common logic extracted into `GameScreen` base class (`game_screen.hpp/.cpp`). Each derived screen is now ~5 lines: a constructor passing `game_` to the base. Further reduction to a class template is possible but deferred.

2. **[DONE]** ~~**PI redefined in four anonymous namespaces**~~ — All occurrences replaced with `std::numbers::pi_v<float>` from `<numbers>` (C++20). Local `PI` constant in `asteroids_game.cpp` removed.

3. **[DONE]** ~~**Text width formula copied everywhere**~~ — Extracted into `textWidth(text, scale)` in `renderer_utils`. All 14 call sites replaced; stale `<cstring>` includes removed from 6 files.

4. **[DONE]** ~~**HUD layout repeated across three games**~~ — Extracted into `drawHUD(renderer, score, lives, maxLives, level)` in `renderer_utils`. Each game now has a single call.

5. **[DONE]** ~~**Game-over overlay repeated**~~ — Extracted into `drawGameOverOverlay(renderer)` in `renderer_utils`. Y-positions standardised across all three games.

6. **[DONE]** ~~**`Color` struct defined twice**~~ — Moved to `renderer_utils.hpp`; local definitions removed from both anonymous namespaces.

7. **[DONE]** ~~**`std::rand()` scattered in four places**~~ — Replaced with `randInt(lo, hi)` in `rng.hpp`, backed by a function-local `std::mt19937` seeded from `std::random_device`. All 8 call sites updated; `<cstdlib>` removed from all game files.

8. **[DONE]** ~~**Background clear repeated in non-game screens**~~ — Extracted into `clearScreen(SDL_Renderer*)` in `renderer_utils`. `Game::clearScreen()` delegates to it; `SplashScreen` and `MenuScreen` call it directly.

9. **[DONE]** ~~**`ROWS`/`COLS` defined twice in Breakout**~~ — Moved to `breakout_constants.hpp` as `BREAKOUT_ROWS`/`BREAKOUT_COLS`. Class member definitions and anonymous namespace definitions both removed.

---

## Dead Code

1. **`BreakoutState::Won` is unreachable** — `nextLevel()` is called directly when all bricks are cleared, so `state_` is never set to `Won`. The `draw()` branch for `Won` renders text that can never appear.

2. **`PongGame::running_`** — Escape is intercepted by `PongScreen` before reaching `PongGame`, so `running_` is only ever set false by `SDL_EVENT_QUIT`, which `App::handleEvent` already handles independently. The member and its `running()` accessor serve no live purpose.

---

## Logic Bugs

1. **Asteroids: level advances on death** — in `AsteroidsGame::update()`, `if (asteroids_.empty()) nextLevel()` fires unconditionally after `checkShipVsAsteroids()`. If the player dies on the same frame the last asteroid is destroyed, `nextLevel()` is called even though `state_` is now `Lost`. The guard should be `if (state_ == AsteroidState::Playing && asteroids_.empty())`.

2. **`ScreenSignal` is never reset to `None`** — once a screen sets its signal, `App::update()` will call `transition()` on the very next frame. Works today, but if `transition()` ever fails silently the stale signal persists forever.

---

## Abstraction Leaks / Coupling

1. **`MessageBus::publish()` is marked `const` but invokes mutable handlers** — misleading const-qualifier; the handlers modify game state (audio output, score).

2. **No unsubscribe on `MessageBus`** — `AudioManager` registers lambdas capturing `this`. The bus outlives `audio_` in `Game`'s destruction order (`audio_` dies first, then `bus_`). Safe today by construction order coincidence; if anything publishes to `bus_` after `audio_` is destroyed the lambda dereferences a dead object.

3. **`Entity::pos` is a public member** — `Ball` and `Paddle` mutate it directly. Encapsulating behind accessors would let `Entity` enforce bounds or add observers later.

4. **`Ball::vel` and `Ball::speed` are public** — `PongGame` reaches into `Ball` to set velocity directly after collisions. The physics response belongs on `Ball`, not on the game.

---

## Naming / Clarity

1. State enum naming is inconsistent — `PongState`, `BreakoutState`, `SIState`, `AsteroidState`: three different conventions; `SIState` uses an abbreviation while the others use full names.
2. Bullet struct naming is inconsistent — `SIBullet` vs `AstBullet`, different prefix styles for the same concept.
3. `PAD_` vs `PADDLE_` prefix inconsistency between Breakout and Pong constants.
4. Serve angle range is unnamed — Pong uses ±22° (`ball.cpp`), Breakout uses ±30° (`breakout_game.cpp`), paddle reflection uses 65° (`breakout_game.cpp`). None are named constants.

---

## Resource Management

1. **`std::vector<float>` allocated per audio tone** — `audio.cpp` allocates a fresh buffer on every brick hit, wall bounce, invader kill, etc. Asteroids can fire six events in one frame (large → 2 medium → 4 small). This per-event heap churn is the most likely source of the intermittent heap corruption crash. A pre-allocated scratch buffer on `AudioManager` would fix it.

2. **Raw `SDL_Renderer*` passed through the whole call chain** — `main` → `App` → `Screen` → `Game` with no ownership semantics. Works fine today but a thin RAII wrapper would make the lifetime contract explicit.

---

## Minor / Low Priority

- `drawNumber()` in `renderer_utils.cpp` indexes `DIGITS[c - '0']` with no bounds check; safe in practice since callers pass `std::to_string()` output, but fragile.
- `MenuItem` array in `menu_screen.cpp` uses `sizeof(ITEMS)/sizeof(ITEMS[0])` instead of `std::size()`.
- `SDL_GetKeyboardState()` return value not null-checked (SDL guarantees non-null, but undocumented at the call site).
- Ball serve angle and Asteroids spawn angle use `std::rand() % N` which has modulo bias; negligible for a game but worth noting.
