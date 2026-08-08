#include "breakout_game.hpp"
#include "breakout_constants.hpp"
#include "renderer_utils.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>

namespace {

constexpr float PAD_W      = 120.0f;
constexpr float PAD_H      = 14.0f;
constexpr float PAD_Y      = WINDOW_H - 56.0f;   // fixed center-y of paddle
constexpr float PAD_SPEED  = 720.0f;

constexpr float BR              = 8.0f;
constexpr float BSPEED          = 480.0f;
constexpr float BSPEED_MAX      = 900.0f;
constexpr float BSPEED_INC      = 10.0f;   // per brick hit
constexpr float LEVEL_SPEED_INC = 40.0f;   // added to base speed each level

constexpr int   ROWS = 6;
constexpr int   COLS = 10;

constexpr float BRICK_GAP    = 6.0f;
constexpr float BRICK_H      = 24.0f;
constexpr float BRICK_TOP    = 80.0f;
constexpr float BRICK_MARGIN = 60.0f;
constexpr float BRICK_W = (WINDOW_W - 2.0f * BRICK_MARGIN - (COLS - 1) * BRICK_GAP) / COLS;

struct Color { Uint8 r, g, b; };
constexpr Color ROW_COLORS[ROWS] = {
    {220,  60,  60},   // red
    {220, 140,  60},   // orange
    {220, 210,  60},   // yellow
    { 60, 180,  60},   // green
    { 60, 120, 220},   // blue
    {140,  60, 220},   // purple
};

// Points per brick, top row worth most
constexpr int ROW_POINTS[ROWS] = {7, 5, 4, 3, 2, 1};

SDL_FRect brickRect(int row, int col)
{
    return {
        BRICK_MARGIN + col * (BRICK_W + BRICK_GAP),
        BRICK_TOP    + row * (BRICK_H + BRICK_GAP),
        BRICK_W,
        BRICK_H,
    };
}

} // namespace

BreakoutGame::BreakoutGame(SDL_Renderer* renderer)
    : Game(renderer)
{
    reset();
}

void BreakoutGame::reset()
{
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            bricks_[r][c] = true;
    bricksLeft_ = ROWS * COLS;
    score_  = 0;
    lives_  = BREAKOUT_LIVES;
    level_  = 1;
    paddle_ = {WINDOW_W / 2.0f, PAD_Y};
    state_  = BreakoutState::Waiting;
    // Ball position set in update() while Waiting
}

void BreakoutGame::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    switch (event.key.key) {
        case SDLK_SPACE:
            if (state_ == BreakoutState::Waiting) {
                const float angle = static_cast<float>(std::rand() % 60 - 30)
                                    * std::numbers::pi_v<float> / 180.0f;
                vel_   = {std::sinf(angle), -std::cosf(angle)};
                speed_ = std::min(BSPEED + static_cast<float>(level_ - 1) * LEVEL_SPEED_INC,
                                  BSPEED_MAX);
                state_ = BreakoutState::Playing;
            } else if (state_ == BreakoutState::Lost || state_ == BreakoutState::Won) {
                reset();
            }
            break;
        default:
            break;
    }
}

void BreakoutGame::update(float dt)
{
    // Paddle always moves (even in Waiting/Won/Lost so player can reposition)
    const bool* keys = SDL_GetKeyboardState(nullptr);
    if (keys[SDL_SCANCODE_LEFT])  paddle_.x -= PAD_SPEED * dt;
    if (keys[SDL_SCANCODE_RIGHT]) paddle_.x += PAD_SPEED * dt;
    paddle_.x = std::clamp(paddle_.x, PAD_W / 2.0f, WINDOW_W - PAD_W / 2.0f);

    if (state_ == BreakoutState::Waiting) {
        // Ball rides on top of paddle until launched
        ball_ = {paddle_.x, PAD_Y - PAD_H / 2.0f - BR - 1.0f};
        return;
    }

    if (state_ != BreakoutState::Playing) return;

    ball_.x += vel_.x * speed_ * dt;
    ball_.y += vel_.y * speed_ * dt;

    checkWalls();
    checkPaddle();
    checkBricks();
}

void BreakoutGame::checkWalls()
{
    if (ball_.x < BR)             { ball_.x = BR;             vel_.x =  std::abs(vel_.x); bus_.publish(Event::WallBounce); }
    if (ball_.x > WINDOW_W - BR)  { ball_.x = WINDOW_W - BR;  vel_.x = -std::abs(vel_.x); bus_.publish(Event::WallBounce); }
    if (ball_.y < BR)             { ball_.y = BR;             vel_.y =  std::abs(vel_.y); bus_.publish(Event::WallBounce); }

    if (ball_.y > WINDOW_H + 20.0f) {
        --lives_;
        bus_.publish(Event::Score);
        if (lives_ <= 0)
            state_ = BreakoutState::Lost;
        else
            state_ = BreakoutState::Waiting;  // re-serve with remaining lives
    }
}

void BreakoutGame::checkPaddle()
{
    if (vel_.y <= 0.0f) return;  // ball moving up — can't hit paddle

    const float px = paddle_.x - PAD_W / 2.0f;
    const float py = PAD_Y     - PAD_H / 2.0f;

    if (ball_.y + BR < py || ball_.y - BR > py + PAD_H) return;
    if (ball_.x + BR < px || ball_.x - BR > px + PAD_W) return;

    // Reflect with angle based on hit position (-1..1 from center)
    const float hitT  = std::clamp((ball_.x - paddle_.x) / (PAD_W / 2.0f), -1.0f, 1.0f);
    const float angle = hitT * (65.0f * std::numbers::pi_v<float> / 180.0f);
    vel_   = {std::sinf(angle), -std::cosf(angle)};
    ball_.y = py - BR;   // push ball above paddle to prevent re-collision
    bus_.publish(Event::PaddleHit);
}

void BreakoutGame::checkBricks()
{
    bool reflected = false;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (!bricks_[r][c]) continue;

            const SDL_FRect br = brickRect(r, c);
            const float nx = std::clamp(ball_.x, br.x, br.x + br.w);
            const float ny = std::clamp(ball_.y, br.y, br.y + br.h);
            const float dx = ball_.x - nx;
            const float dy = ball_.y - ny;

            if (dx * dx + dy * dy > BR * BR) continue;

            bricks_[r][c] = false;
            --bricksLeft_;
            score_ += ROW_POINTS[r];
            bus_.publish(Event::PaddleHit);

            if (!reflected) {
                reflected = true;
                // Reflect on the axis of least penetration
                if (std::abs(dx) > std::abs(dy))
                    vel_.x = -vel_.x;
                else
                    vel_.y = -vel_.y;
                speed_ = std::min(speed_ + BSPEED_INC, BSPEED_MAX);
            }
        }
    }

    if (bricksLeft_ == 0) {
        bus_.publish(Event::Win);
        nextLevel();
    }
}

void BreakoutGame::nextLevel()
{
    ++level_;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            bricks_[r][c] = true;
    bricksLeft_ = ROWS * COLS;
    state_ = BreakoutState::Waiting;
}

void BreakoutGame::draw() const
{
    clearScreen();

    // Bricks
    for (int r = 0; r < ROWS; ++r) {
        const Color& col = ROW_COLORS[r];
        SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 255);
        for (int c = 0; c < COLS; ++c) {
            if (!bricks_[r][c]) continue;
            SDL_FRect rect = brickRect(r, c);
            SDL_RenderFillRect(renderer_, &rect);
        }
    }

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    // Score (top-right)
    drawNumber(renderer_, score_, WINDOW_W - 160.0f, 20.0f, 8.0f);

    // Level (top-centre): "LVL " prefix = 4 chars × 32 px = 128 px wide
    drawText(renderer_, "LVL", WINDOW_W / 2.0f - 64.0f, 20.0f, 8.0f);
    drawNumber(renderer_, level_, WINDOW_W / 2.0f + 64.0f, 20.0f, 8.0f);

    // Lives pips (top-left)
    constexpr float PIP = 14.0f;
    constexpr float GAP =  8.0f;
    for (int i = 0; i < BREAKOUT_LIVES; ++i) {
        if (i >= lives_) continue;
        SDL_FRect pip{40.0f + static_cast<float>(i) * (PIP + GAP), 20.0f, PIP, PIP};
        SDL_RenderFillRect(renderer_, &pip);
    }

    // Paddle
    SDL_FRect padRect{paddle_.x - PAD_W / 2.0f, PAD_Y - PAD_H / 2.0f, PAD_W, PAD_H};
    SDL_RenderFillRect(renderer_, &padRect);

    // Ball
    SDL_FRect ballRect{ball_.x - BR, ball_.y - BR, BR * 2.0f, BR * 2.0f};
    SDL_RenderFillRect(renderer_, &ballRect);

    // Overlays
    if (state_ == BreakoutState::Waiting) {
        constexpr float sc = 6.0f;
        const char* msg = "SPACE TO LAUNCH";
        const float w = static_cast<float>(std::strlen(msg)) * 4.0f * sc;
        drawText(renderer_, msg, WINDOW_W / 2.0f - w / 2.0f, PAD_Y - 80.0f, sc);

    } else if (state_ == BreakoutState::Lost) {
        constexpr float sc1 = 7.0f, sc2 = 5.0f;
        const char* line1 = "GAME OVER";
        const char* line2 = "SPACE RETRY  ESC MENU";
        const float w1 = static_cast<float>(std::strlen(line1)) * 4.0f * sc1;
        const float w2 = static_cast<float>(std::strlen(line2)) * 4.0f * sc2;
        drawText(renderer_, line1, WINDOW_W / 2.0f - w1 / 2.0f, WINDOW_H / 2.0f + 60.0f, sc1);
        drawText(renderer_, line2, WINDOW_W / 2.0f - w2 / 2.0f, WINDOW_H / 2.0f + 120.0f, sc2);

    } else if (state_ == BreakoutState::Won) {
        constexpr float sc1 = 7.0f, sc2 = 5.0f;
        const char* line1 = "YOU WIN";
        const char* line2 = "SPACE PLAY AGAIN  ESC MENU";
        const float w1 = static_cast<float>(std::strlen(line1)) * 4.0f * sc1;
        const float w2 = static_cast<float>(std::strlen(line2)) * 4.0f * sc2;
        drawText(renderer_, line1, WINDOW_W / 2.0f - w1 / 2.0f, WINDOW_H / 2.0f + 60.0f, sc1);
        drawText(renderer_, line2, WINDOW_W / 2.0f - w2 / 2.0f, WINDOW_H / 2.0f + 120.0f, sc2);
    }

    SDL_RenderPresent(renderer_);
}
