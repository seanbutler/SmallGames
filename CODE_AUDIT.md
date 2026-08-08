# SmallGames Code Audit

## Duplication / DRY

1. **[DONE]** ~~**All four `XxxScreen` classes are identical boilerplate**~~ — Common logic extracted into `GameScreen` base class (`game_screen.hpp/.cpp`). Each derived screen is now ~5 lines: a constructor passing `game_` to the base. Further reduction to a class template is possible but deferred.

2. **[DONE]** ~~**PI redefined in four anonymous namespaces**~~ — All occurrences replaced with `std::numbers::pi_v<float>` from `<numbers>` (C++20). Local `PI` constant in `asteroids_game.cpp` removed.

3. **Text width formula copied everywhere** — `static_cast<float>(std::strlen(msg)) * 4.0f * scale` appears in every `draw()` across all files. Should be a `textWidth(const char*, float)` free function in `renderer_utils`.

4. **HUD layout repeated across three games** — score top-right, lives pips top-left, LVL top-centre with identical magic numbers in Breakout, Space Invaders, and Asteroids. A `drawHUD(renderer, score, lives, maxLives, level)` helper would unify them.

5. **Game-over overlay repeated** — the "GAME OVER" / "SPACE RETRY ESC MENU" two-line centred overlay is copy-pasted verbatim into Breakout, Space Invaders, and Asteroids.

6. **`Color` struct defined twice** — identical `struct Color { Uint8 r, g, b; }` in `breakout_game.cpp` and `spaceinvaders_game.cpp` anonymous namespaces.

7. **`std::rand()` scattered in four places** — `ball.cpp`, `breakout_game.cpp`, `spaceinvaders_game.cpp`, `asteroids_game.cpp` all call `std::rand()` with no seed, so every run produces identical random patterns. Should be a single seeded `std::mt19937` in a shared utility.

8. **Background clear repeated in non-game screens** — `SDL_SetRenderDrawColor(renderer_, 20, 20, 28, 255)` + `SDL_RenderClear` appear in `SplashScreen::draw()` and `MenuScreen::draw()`. `Game::clearScreen()` already centralises this but `IScreen` has no equivalent.

9. **`ROWS`/`COLS` defined twice in Breakout** — once as `static constexpr int` members of `BreakoutGame`, and again in the `breakout_game.cpp` anonymous namespace. In sync by coincidence.

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
