# SmallGames Code Audit

## Duplication / DRY

**PI repeated everywhere** — `3.14159265f` appears in `game.cpp`, `ball.cpp`, `audio.cpp`, `breakout_game.cpp`. Should be one named constant.

**`PongScreen` and `BreakoutScreen` are near-identical** — both intercept Escape, delegate `update`/`draw` to a game object, manage a `signal_` member. A template or base class `GameScreen<T>` would eliminate ~40 lines of copy-paste.

**Text width formula copied everywhere** — `std::strlen(msg) * 4.0f * scale` appears in `game.cpp`, `splash_screen.cpp`, `menu_screen.cpp`, `breakout_game.cpp`. Should be a `textWidth(const char*, float)` utility in `renderer_utils`.

**Circle-vs-rect collision implemented twice** — once in `game.cpp::circleHitsRect()` and again inline in `breakout_game.cpp::checkBricks()`.

**Background/foreground colour setup repeated** — the same two `SDL_SetRenderDrawColor` calls appear in every `draw()` across all four screen types.

---

## Scattered / Missing Constants

**Breakout physics constants are local to the `.cpp` anonymous namespace** — `PAD_W`, `BSPEED`, `BRICK_GAP`, etc. are inaccessible from tests or tooling and inconsistent with Pong constants living in `constants.hpp`.

**Ball serve angle range differs between games** — Pong uses ±22° (`ball.cpp:37`), Breakout uses ±30° (`breakout_game.cpp:82`), paddle reflection uses 65° (`breakout_game.cpp:149`). None are named.

**`std::rand()` used in two places** — `ball.cpp:37`, `breakout_game.cpp:82`. Should use `<random>` with a seeded `std::mt19937`.

---

## State Machine Design

**Transitions are scattered** — in `Game`, state changes happen in both `handleEvent()` and `update()` with no central transition table. Adding a state means touching multiple methods.

**`ScreenSignal` is never reset to `None`** — once a screen sets its signal, `App::update()` will call `transition()` on the very next frame. Works today, but if `transition()` ever fails silently the stale signal persists forever.

**`BreakoutState::Won` exists in the enum but is now unreachable** — `nextLevel()` was wired in and the `Won` branch in `draw()` is dead code.

---

## Abstraction Leaks / Coupling

**`Entity::pos` is a public member** — `Ball`, `Paddle`, and `BreakoutGame` all mutate it directly. Encapsulating it behind accessors would let `Entity` enforce bounds or add observers later.

**`Ball::vel` and `Ball::speed` are public** — `Game` reaches into `Ball` to set velocity directly after collisions (`game.cpp:110–113`). The physics response belongs on `Ball`, not in `Game`.

**`MessageBus::publish()` is marked `const` but invokes mutable handlers** — misleading const-qualifier.

**No unsubscribe on `MessageBus`** — `AudioManager` registers lambdas that capture `this`. If `AudioManager` is destroyed and the bus outlives it the handlers dangle. Currently safe by construction order, but fragile.

---

## Naming / Clarity

- `game.cpp` / `game.hpp` should be `pong_game.cpp` / `pong_game.hpp` — named generically because Pong was the first game, before the multi-game architecture existed. Breakout follows the explicit `breakout_game.*` pattern; Pong should match.
- `rel` in `game.cpp:108` — means "relative hit position", not obvious.
- `col` in `breakout_game.cpp:211` — ambiguous between "colour" and "column".
- `PAD_` vs `PADDLE_` prefix inconsistency across the two games.
- `GameState` vs `BreakoutState` vs `ScreenSignal` — three different naming conventions for the same concept.

---

## Resource Management

**`std::vector<float>` allocated per audio tone** — `audio.cpp:33` allocates a fresh buffer on every brick hit, wall bounce, etc. A small pre-allocated scratch buffer would remove the per-event heap allocation.

**Raw `SDL_Renderer*` passed through the whole call chain** — `main` → `App` → `Screen` → `Game` with no ownership semantics. Works fine today but a thin RAII wrapper or `shared_ptr` would make the lifetime contract explicit.

---

## Minor / Low Priority

- `drawNumber()` in `renderer_utils.cpp:103` indexes `DIGITS[c - '0']` with no bounds check; `drawText()` is safe but `drawNumber()` is not.
- `MenuItem` array in `menu_screen.cpp` uses `sizeof(ITEMS)/sizeof(ITEMS[0])` instead of `std::size()` or `std::array`.
- `SDL_GetKeyboardState()` return value not null-checked (SDL guarantees non-null, but it's undocumented at the call site).
