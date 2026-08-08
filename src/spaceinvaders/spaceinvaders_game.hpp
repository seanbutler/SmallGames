#pragma once
#include "game.hpp"
#include "math.hpp"
#include "spaceinvaders_constants.hpp"

enum class SIState { Waiting, Playing, Lost };

struct SIBullet {
    Vec2 pos{};
    bool active{false};
};

class SpaceInvadersGame : public Game {
public:
    explicit SpaceInvadersGame(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;

private:
    void reset();
    void nextLevel();
    void updateSwarm(float dt);
    void updateBullets(float dt);
    void spawnEnemyBullet();
    void checkPlayerBulletVsInvaders();
    void checkEnemyBulletsVsPlayer();
    bool anyInvaderReachedPlayer() const;
    int  leftmostAliveCol() const;
    int  rightmostAliveCol() const;

    bool     invaders_[SI_ROWS][SI_COLS]{};
    int      alive_{};
    float    swarmX_{SI_SWARM_START_X};
    float    swarmY_{SI_SWARM_START_Y};
    float    marchDir_{1.0f};

    float    playerX_{WINDOW_W / 2.0f};
    int      lives_{SI_LIVES};
    int      score_{};
    int      level_{1};

    SIBullet playerBullet_{};
    SIBullet enemyBullets_[SI_MAX_ENEMY_BULLETS]{};
    float    shootTimer_{};

    SIState  state_{SIState::Waiting};
};
