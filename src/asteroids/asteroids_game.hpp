#pragma once
#include "game.hpp"
#include "math.hpp"
#include "asteroids_constants.hpp"
#include <vector>

enum class AsteroidState { Waiting, Playing, Lost };

struct AstBullet {
    Vec2  pos{};
    Vec2  vel{};
    float life{};
    bool  active{false};
};

struct Asteroid {
    Vec2  pos{};
    Vec2  vel{};
    float radius{};
    int   size{};       // 2=large, 1=medium, 0=small
    bool  alive{true};
    float vx[AST_VERTS]{};
    float vy[AST_VERTS]{};
};

class AsteroidsGame : public Game {
public:
    explicit AsteroidsGame(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;

private:
    void reset();
    void nextLevel();
    void spawnInitialAsteroids();
    void spawnAsteroid(Vec2 pos, Vec2 vel, int size);
    void respawnShip();
    void updateShip(float dt);
    void updateBullets(float dt);
    void updateAsteroids(float dt);
    void checkBulletVsAsteroids();
    void checkShipVsAsteroids();

    Vec2  shipPos_{WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    Vec2  shipVel_{};
    float shipAngle_{};   // initialised in reset()
    float invincible_{AST_INVINCIBLE_TIME};
    bool  thrusting_{false};

    int   lives_{AST_LIVES};
    int   score_{};
    int   level_{1};

    AstBullet             bullets_[AST_MAX_BULLETS]{};
    std::vector<Asteroid> asteroids_{};

    AsteroidState state_{AsteroidState::Waiting};
};
