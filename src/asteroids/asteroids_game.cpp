#include "asteroids_game.hpp"
#include "renderer_utils.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <numbers>

namespace {

float asteroidRadius(int size)
{
    if (size == 2) return AST_RAD_LARGE;
    if (size == 1) return AST_RAD_MEDIUM;
    return AST_RAD_SMALL;
}

float asteroidSpeed(int size, int level)
{
    const float base = (size == 2) ? AST_SPEED_LARGE
                     : (size == 1) ? AST_SPEED_MEDIUM
                                   : AST_SPEED_SMALL;
    return base + static_cast<float>(level - 1) * AST_LEVEL_SPEED_INC;
}

int scoreForSize(int size)
{
    if (size == 2) return AST_SCORE_LARGE;
    if (size == 1) return AST_SCORE_MEDIUM;
    return AST_SCORE_SMALL;
}

void generateVerts(float vx[], float vy[], float radius)
{
    for (int i = 0; i < AST_VERTS; ++i) {
        const float base  = static_cast<float>(i) * (2.0f * std::numbers::pi_v<float> / AST_VERTS);
        const float jit   = static_cast<float>(std::rand() % 31 - 15) * (std::numbers::pi_v<float> / 180.0f);
        const float scale = 0.70f + static_cast<float>(std::rand() % 31) / 100.0f;
        vx[i] = std::cos(base + jit) * radius * scale;
        vy[i] = std::sin(base + jit) * radius * scale;
    }
}

void drawPoly(SDL_Renderer* r, float cx, float cy,
              const float vx[], const float vy[], int n)
{
    for (int i = 0; i < n; ++i) {
        const int j = (i + 1) % n;
        SDL_RenderLine(r, cx + vx[i], cy + vy[i], cx + vx[j], cy + vy[j]);
    }
}

void splitAsteroid(std::vector<Asteroid>& out, const Asteroid& src, int level)
{
    const int   newSize  = src.size - 1;
    const float newSpeed = asteroidSpeed(newSize, level);
    const float base     = std::atan2(src.vel.y, src.vel.x);

    for (int i = -1; i <= 1; i += 2) {
        const float a = base + static_cast<float>(i) * (std::numbers::pi_v<float> / 5.0f);
        Asteroid child{};
        child.pos    = src.pos;
        child.vel    = {std::cos(a) * newSpeed, std::sin(a) * newSpeed};
        child.size   = newSize;
        child.radius = asteroidRadius(newSize);
        child.alive  = true;
        generateVerts(child.vx, child.vy, child.radius);
        out.push_back(child);
    }
}

Vec2 wrapPos(Vec2 p, float margin)
{
    const float W = static_cast<float>(WINDOW_W);
    const float H = static_cast<float>(WINDOW_H);
    if (p.x < -margin)    p.x += W + margin * 2;
    if (p.x > W + margin) p.x -= W + margin * 2;
    if (p.y < -margin)    p.y += H + margin * 2;
    if (p.y > H + margin) p.y -= H + margin * 2;
    return p;
}

} // namespace

AsteroidsGame::AsteroidsGame(SDL_Renderer* renderer)
    : Game(renderer)
{
    reset();
}

void AsteroidsGame::reset()
{
    shipPos_    = {WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    shipVel_    = {};
    shipAngle_  = -std::numbers::pi_v<float> / 2.0f;   // pointing up
    invincible_ = AST_INVINCIBLE_TIME;
    thrusting_  = false;

    lives_ = AST_LIVES;
    score_ = 0;
    level_ = 1;

    for (auto& b : bullets_) b.active = false;
    asteroids_.clear();
    spawnInitialAsteroids();

    state_ = AsteroidState::Waiting;
}

void AsteroidsGame::nextLevel()
{
    ++level_;
    shipPos_    = {WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    shipVel_    = {};
    shipAngle_  = -std::numbers::pi_v<float> / 2.0f;
    invincible_ = AST_INVINCIBLE_TIME;
    thrusting_  = false;

    for (auto& b : bullets_) b.active = false;
    asteroids_.clear();
    spawnInitialAsteroids();

    state_ = AsteroidState::Waiting;
}

void AsteroidsGame::spawnInitialAsteroids()
{
    const int count = std::min(AST_START_LARGE + level_ - 1, AST_MAX_LARGE);
    for (int i = 0; i < count; ++i) {
        Vec2 pos{};
        for (int attempt = 0; attempt < 20; ++attempt) {
            pos = {static_cast<float>(std::rand() % WINDOW_W),
                   static_cast<float>(std::rand() % WINDOW_H)};
            const float dx = pos.x - WINDOW_W / 2.0f;
            const float dy = pos.y - WINDOW_H / 2.0f;
            if (dx*dx + dy*dy >= AST_SPAWN_SAFE_DIST * AST_SPAWN_SAFE_DIST)
                break;
        }
        const float angle = static_cast<float>(std::rand() % 1000) * (2.0f * std::numbers::pi_v<float> / 1000.0f);
        const float speed = asteroidSpeed(2, level_);
        spawnAsteroid(pos, {std::cos(angle) * speed, std::sin(angle) * speed}, 2);
    }
}

void AsteroidsGame::spawnAsteroid(Vec2 pos, Vec2 vel, int size)
{
    Asteroid ast{};
    ast.pos    = pos;
    ast.vel    = vel;
    ast.size   = size;
    ast.radius = asteroidRadius(size);
    ast.alive  = true;
    generateVerts(ast.vx, ast.vy, ast.radius);
    asteroids_.push_back(ast);
}

void AsteroidsGame::respawnShip()
{
    shipPos_    = {WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    shipVel_    = {};
    shipAngle_  = -std::numbers::pi_v<float> / 2.0f;
    invincible_ = AST_INVINCIBLE_TIME;
    thrusting_  = false;
    for (auto& b : bullets_) b.active = false;
}

void AsteroidsGame::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    switch (event.key.key) {
        case SDLK_SPACE:
            if (state_ == AsteroidState::Waiting) {
                state_ = AsteroidState::Playing;
            } else if (state_ == AsteroidState::Playing) {
                for (auto& b : bullets_) {
                    if (!b.active) {
                        b.pos    = {shipPos_.x + std::cos(shipAngle_) * AST_SHIP_RADIUS,
                                    shipPos_.y + std::sin(shipAngle_) * AST_SHIP_RADIUS};
                        b.vel    = {std::cos(shipAngle_) * AST_BULLET_SPEED + shipVel_.x,
                                    std::sin(shipAngle_) * AST_BULLET_SPEED + shipVel_.y};
                        b.life   = AST_BULLET_LIFE;
                        b.active = true;
                        bus_.publish(Event::PaddleHit);
                        break;
                    }
                }
            } else if (state_ == AsteroidState::Lost) {
                reset();
            }
            break;
        default:
            break;
    }
}

void AsteroidsGame::update(float dt)
{
    if (state_ == AsteroidState::Lost) return;

    updateShip(dt);
    updateAsteroids(dt);

    if (state_ == AsteroidState::Playing) {
        if (invincible_ > 0.0f) invincible_ -= dt;
        updateBullets(dt);
        checkBulletVsAsteroids();
        if (invincible_ <= 0.0f)
            checkShipVsAsteroids();
        if (asteroids_.empty())
            nextLevel();
    }
}

void AsteroidsGame::updateShip(float dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_LEFT])  shipAngle_ -= AST_SHIP_ROTATE * dt;
    if (keys[SDL_SCANCODE_RIGHT]) shipAngle_ += AST_SHIP_ROTATE * dt;

    if (state_ != AsteroidState::Playing) {
        thrusting_ = false;
        return;
    }

    thrusting_ = keys[SDL_SCANCODE_UP] != 0;
    if (thrusting_) {
        shipVel_.x += std::cos(shipAngle_) * AST_SHIP_ACCEL * dt;
        shipVel_.y += std::sin(shipAngle_) * AST_SHIP_ACCEL * dt;
    }

    const float drag = 1.0f - AST_SHIP_DRAG * dt;
    shipVel_.x *= drag;
    shipVel_.y *= drag;

    shipPos_.x += shipVel_.x * dt;
    shipPos_.y += shipVel_.y * dt;
    shipPos_ = wrapPos(shipPos_, AST_SHIP_RADIUS);
}

void AsteroidsGame::updateBullets(float dt)
{
    for (auto& b : bullets_) {
        if (!b.active) continue;
        b.pos.x += b.vel.x * dt;
        b.pos.y += b.vel.y * dt;
        b.pos    = wrapPos(b.pos, 4.0f);
        b.life  -= dt;
        if (b.life <= 0.0f) b.active = false;
    }
}

void AsteroidsGame::updateAsteroids(float dt)
{
    for (auto& ast : asteroids_) {
        ast.pos.x += ast.vel.x * dt;
        ast.pos.y += ast.vel.y * dt;
        ast.pos    = wrapPos(ast.pos, ast.radius);
    }
}

void AsteroidsGame::checkBulletVsAsteroids()
{
    std::vector<Asteroid> spawns;

    for (auto& b : bullets_) {
        if (!b.active) continue;
        for (auto& ast : asteroids_) {
            if (!ast.alive) continue;
            const float dx = b.pos.x - ast.pos.x;
            const float dy = b.pos.y - ast.pos.y;
            if (dx*dx + dy*dy < ast.radius * ast.radius) {
                b.active  = false;
                ast.alive = false;
                score_   += scoreForSize(ast.size);
                bus_.publish(Event::Score);
                if (ast.size > 0)
                    splitAsteroid(spawns, ast, level_);
                break;
            }
        }
    }

    asteroids_.erase(
        std::remove_if(asteroids_.begin(), asteroids_.end(),
                       [](const Asteroid& a){ return !a.alive; }),
        asteroids_.end());

    for (auto& s : spawns)
        asteroids_.push_back(std::move(s));
}

void AsteroidsGame::checkShipVsAsteroids()
{
    for (const auto& ast : asteroids_) {
        const float dx   = shipPos_.x - ast.pos.x;
        const float dy   = shipPos_.y - ast.pos.y;
        const float minD = AST_SHIP_RADIUS + ast.radius;
        if (dx*dx + dy*dy < minD * minD) {
            --lives_;
            bus_.publish(Event::Score);
            if (lives_ <= 0)
                state_ = AsteroidState::Lost;
            else
                respawnShip();
            return;
        }
    }
}

void AsteroidsGame::draw() const
{
    clearScreen();

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    drawHUD(renderer_, score_, lives_, AST_LIVES, level_);

    // Asteroids
    for (const auto& ast : asteroids_)
        drawPoly(renderer_, ast.pos.x, ast.pos.y, ast.vx, ast.vy, AST_VERTS);

    // Ship — blink while invincible
    const bool showShip = state_ != AsteroidState::Lost &&
                          (invincible_ <= 0.0f || static_cast<int>(invincible_ * 8) % 2 == 0);
    if (showShip) {
        const float R  = AST_SHIP_RADIUS;
        const float fx = shipPos_.x + std::cos(shipAngle_) * R * 1.5f;
        const float fy = shipPos_.y + std::sin(shipAngle_) * R * 1.5f;
        const float lx = shipPos_.x + std::cos(shipAngle_ + 2.4f) * R;
        const float ly = shipPos_.y + std::sin(shipAngle_ + 2.4f) * R;
        const float rx = shipPos_.x + std::cos(shipAngle_ - 2.4f) * R;
        const float ry = shipPos_.y + std::sin(shipAngle_ - 2.4f) * R;

        SDL_RenderLine(renderer_, fx, fy, lx, ly);
        SDL_RenderLine(renderer_, lx, ly, rx, ry);
        SDL_RenderLine(renderer_, rx, ry, fx, fy);

        if (thrusting_) {
            // Flame: small V from the back of the ship
            const float blx = shipPos_.x + std::cos(shipAngle_ + 2.4f) * R * 0.5f;
            const float bly = shipPos_.y + std::sin(shipAngle_ + 2.4f) * R * 0.5f;
            const float brx = shipPos_.x + std::cos(shipAngle_ - 2.4f) * R * 0.5f;
            const float bry = shipPos_.y + std::sin(shipAngle_ - 2.4f) * R * 0.5f;
            const float ftx = shipPos_.x + std::cos(shipAngle_ + std::numbers::pi_v<float>) * R * 1.4f;
            const float fty = shipPos_.y + std::sin(shipAngle_ + std::numbers::pi_v<float>) * R * 1.4f;
            SDL_SetRenderDrawColor(renderer_, 220, 120, 40, 255);
            SDL_RenderLine(renderer_, blx, bly, ftx, fty);
            SDL_RenderLine(renderer_, brx, bry, ftx, fty);
            SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
        }
    }

    // Bullets
    SDL_SetRenderDrawColor(renderer_, 255, 220, 50, 255);
    for (const auto& b : bullets_) {
        if (!b.active) continue;
        SDL_FRect br{b.pos.x - 2.0f, b.pos.y - 2.0f, 4.0f, 4.0f};
        SDL_RenderFillRect(renderer_, &br);
    }

    // Overlays
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    if (state_ == AsteroidState::Waiting) {
        constexpr float sc = 6.0f;
        const char* msg = "SPACE TO START";
        const float w = static_cast<float>(std::strlen(msg)) * 4.0f * sc;
        drawText(renderer_, msg, WINDOW_W / 2.0f - w / 2.0f, WINDOW_H * 0.72f, sc);
    } else if (state_ == AsteroidState::Lost) {
        constexpr float sc1 = 7.0f, sc2 = 5.0f;
        const char* line1 = "GAME OVER";
        const char* line2 = "SPACE RETRY  ESC MENU";
        const float w1 = static_cast<float>(std::strlen(line1)) * 4.0f * sc1;
        const float w2 = static_cast<float>(std::strlen(line2)) * 4.0f * sc2;
        drawText(renderer_, line1, WINDOW_W / 2.0f - w1 / 2.0f, WINDOW_H / 2.0f - 30.0f, sc1);
        drawText(renderer_, line2, WINDOW_W / 2.0f - w2 / 2.0f, WINDOW_H / 2.0f + 50.0f, sc2);
    }

    SDL_RenderPresent(renderer_);
}
