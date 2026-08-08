#include "spaceinvaders_game.hpp"
#include "renderer_utils.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {

struct Color { Uint8 r, g, b; };

constexpr Color ROW_COLORS[SI_ROWS] = {
    {  0, 220, 220},   // row 0: cyan   (30 pts)
    { 60, 220,  60},   // row 1: green  (20 pts)
    { 60, 220,  60},   // row 2: green  (20 pts)
    {220, 200,  60},   // row 3: yellow (10 pts)
    {220, 200,  60},   // row 4: yellow (10 pts)
};

constexpr int ROW_POINTS[SI_ROWS] = {30, 20, 20, 10, 10};

int invaderType(int row)
{
    if (row == 0) return 0;
    if (row <= 2) return 1;
    return 2;
}

// Draws a single invader shape within a SI_INVADER_W × SI_INVADER_H box at (x, y).
void drawInvader(SDL_Renderer* r, float x, float y, int type)
{
    const float W = SI_INVADER_W;
    const float H = SI_INVADER_H;

    if (type == 0) {
        // Top row: narrow body with two antennas and feet
        SDL_FRect body{x + 5, y + 5, W - 10, H - 9};
        SDL_FRect ant1{x + 3, y + 1, 4, 4};
        SDL_FRect ant2{x + W - 7, y + 1, 4, 4};
        SDL_FRect ft1 {x + 3, y + H - 4, 4, 4};
        SDL_FRect ft2 {x + W - 7, y + H - 4, 4, 4};
        SDL_RenderFillRect(r, &body);
        SDL_RenderFillRect(r, &ant1);
        SDL_RenderFillRect(r, &ant2);
        SDL_RenderFillRect(r, &ft1);
        SDL_RenderFillRect(r, &ft2);
    } else if (type == 1) {
        // Middle rows: wider body with side claws
        SDL_FRect body{x + 2, y + 4, W - 4, H - 9};
        SDL_FRect cl1 {x,     y + 7, 4, 7};
        SDL_FRect cl2 {x + W - 4, y + 7, 4, 7};
        SDL_FRect ft1 {x + 2, y + H - 4, 5, 4};
        SDL_FRect ft2 {x + W - 7, y + H - 4, 5, 4};
        SDL_RenderFillRect(r, &body);
        SDL_RenderFillRect(r, &cl1);
        SDL_RenderFillRect(r, &cl2);
        SDL_RenderFillRect(r, &ft1);
        SDL_RenderFillRect(r, &ft2);
    } else {
        // Bottom rows: squat body with four legs
        SDL_FRect body{x + 1, y + 3, W - 2, H - 7};
        SDL_RenderFillRect(r, &body);
        for (int i = 0; i < 4; ++i) {
            SDL_FRect leg{x + 1 + static_cast<float>(i) * 7.0f, y + H - 4, 4, 4};
            SDL_RenderFillRect(r, &leg);
        }
    }
}

} // namespace

SpaceInvadersGame::SpaceInvadersGame(SDL_Renderer* renderer)
    : Game(renderer)
{
    reset();
}

void SpaceInvadersGame::reset()
{
    for (int r = 0; r < SI_ROWS; ++r)
        for (int c = 0; c < SI_COLS; ++c)
            invaders_[r][c] = true;
    alive_ = SI_ROWS * SI_COLS;

    swarmX_   = SI_SWARM_START_X;
    swarmY_   = SI_SWARM_START_Y;
    marchDir_ = 1.0f;

    playerX_             = WINDOW_W / 2.0f;
    lives_               = SI_LIVES;
    score_               = 0;
    level_               = 1;
    playerBullet_.active = false;
    for (auto& b : enemyBullets_) b.active = false;
    shootTimer_ = 0.0f;

    state_ = SIState::Waiting;
}

void SpaceInvadersGame::nextLevel()
{
    ++level_;
    for (int r = 0; r < SI_ROWS; ++r)
        for (int c = 0; c < SI_COLS; ++c)
            invaders_[r][c] = true;
    alive_ = SI_ROWS * SI_COLS;

    swarmX_   = SI_SWARM_START_X;
    swarmY_   = SI_SWARM_START_Y;
    marchDir_ = 1.0f;

    playerBullet_.active = false;
    for (auto& b : enemyBullets_) b.active = false;
    shootTimer_ = 0.0f;

    state_ = SIState::Waiting;
}

void SpaceInvadersGame::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    switch (event.key.key) {
        case SDLK_SPACE:
            if (state_ == SIState::Waiting) {
                state_ = SIState::Playing;
            } else if (state_ == SIState::Playing && !playerBullet_.active) {
                playerBullet_ = {{playerX_, SI_PLAYER_Y - SI_PLAYER_H / 2.0f - SI_PBULLET_H}, true};
                bus_.publish(Event::PaddleHit);
            } else if (state_ == SIState::Lost) {
                reset();
            }
            break;
        default:
            break;
    }
}

void SpaceInvadersGame::update(float dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    if (keys[SDL_SCANCODE_LEFT])
        playerX_ -= SI_PLAYER_SPEED * dt;
    if (keys[SDL_SCANCODE_RIGHT])
        playerX_ += SI_PLAYER_SPEED * dt;
    playerX_ = std::clamp(playerX_, SI_PLAYER_W / 2.0f, WINDOW_W - SI_PLAYER_W / 2.0f);

    if (state_ != SIState::Playing) return;

    updateSwarm(dt);
    updateBullets(dt);
    checkPlayerBulletVsInvaders();
    checkEnemyBulletsVsPlayer();

    if (anyInvaderReachedPlayer())
        state_ = SIState::Lost;
}

void SpaceInvadersGame::updateSwarm(float dt)
{
    const float speed = SI_MARCH_BASE
                      + static_cast<float>(level_ - 1) * SI_LEVEL_MARCH_INC
                      + static_cast<float>(SI_ROWS * SI_COLS - alive_) * SI_MARCH_INC;
    swarmX_ += marchDir_ * speed * dt;

    const int lc = leftmostAliveCol();
    const int rc = rightmostAliveCol();
    if (lc < 0) return;

    const float leftEdge  = swarmX_ + static_cast<float>(lc) * SI_CELL_W;
    const float rightEdge = swarmX_ + static_cast<float>(rc) * SI_CELL_W + SI_INVADER_W;

    if (marchDir_ > 0.0f && rightEdge >= WINDOW_W - SI_WALL_MARGIN) {
        marchDir_ = -1.0f;
        swarmY_  += SI_STEP_DOWN;
    } else if (marchDir_ < 0.0f && leftEdge <= SI_WALL_MARGIN) {
        marchDir_ = 1.0f;
        swarmY_  += SI_STEP_DOWN;
    }
}

void SpaceInvadersGame::updateBullets(float dt)
{
    if (playerBullet_.active) {
        playerBullet_.pos.y -= SI_PBULLET_SPEED * dt;
        if (playerBullet_.pos.y < 0)
            playerBullet_.active = false;
    }

    for (auto& b : enemyBullets_) {
        if (!b.active) continue;
        b.pos.y += SI_EBULLET_SPEED * dt;
        if (b.pos.y > WINDOW_H)
            b.active = false;
    }

    const float interval = std::max(
        SI_SHOOT_INTERVAL_MIN,
        SI_SHOOT_INTERVAL_BASE
            - static_cast<float>(level_ - 1) * SI_SHOOT_INTERVAL_LDEC
            - static_cast<float>(SI_ROWS * SI_COLS - alive_) * SI_SHOOT_INTERVAL_DEC);
    shootTimer_ += dt;
    if (shootTimer_ >= interval) {
        shootTimer_ = 0.0f;
        spawnEnemyBullet();
    }
}

void SpaceInvadersGame::spawnEnemyBullet()
{
    SIBullet* slot = nullptr;
    for (auto& b : enemyBullets_)
        if (!b.active) { slot = &b; break; }
    if (!slot) return;

    // Gather the bottom-most alive invader per column
    int validCols[SI_COLS];
    int bottomRow[SI_COLS];
    int count = 0;
    for (int c = 0; c < SI_COLS; ++c) {
        for (int r = SI_ROWS - 1; r >= 0; --r) {
            if (invaders_[r][c]) {
                validCols[count] = c;
                bottomRow[count] = r;
                ++count;
                break;
            }
        }
    }
    if (count == 0) return;

    const int pick = std::rand() % count;
    const int c    = validCols[pick];
    const int r    = bottomRow[pick];

    slot->pos    = {swarmX_ + static_cast<float>(c) * SI_CELL_W + SI_INVADER_W / 2.0f,
                    swarmY_ + static_cast<float>(r) * SI_CELL_H + SI_INVADER_H};
    slot->active = true;
}

void SpaceInvadersGame::checkPlayerBulletVsInvaders()
{
    if (!playerBullet_.active) return;

    const float bx = playerBullet_.pos.x;
    const float by = playerBullet_.pos.y;

    for (int r = 0; r < SI_ROWS; ++r) {
        for (int c = 0; c < SI_COLS; ++c) {
            if (!invaders_[r][c]) continue;

            // Invader rect uses cell-margin offset to match drawInvader
            const float cellX = swarmX_ + static_cast<float>(c) * SI_CELL_W;
            const float cellY = swarmY_ + static_cast<float>(r) * SI_CELL_H;
            const float ix    = cellX + (SI_CELL_W - SI_INVADER_W) / 2.0f;
            const float iy    = cellY + (SI_CELL_H - SI_INVADER_H) / 2.0f;

            if (bx >= ix && bx <= ix + SI_INVADER_W &&
                by >= iy && by <= iy + SI_INVADER_H) {
                invaders_[r][c]      = false;
                --alive_;
                score_              += ROW_POINTS[r];
                playerBullet_.active = false;
                bus_.publish(Event::Score);

                if (alive_ == 0) {
                    bus_.publish(Event::Win);
                    nextLevel();
                }
                return;
            }
        }
    }
}

void SpaceInvadersGame::checkEnemyBulletsVsPlayer()
{
    const float px = playerX_ - SI_PLAYER_W / 2.0f;
    const float py = SI_PLAYER_Y - SI_PLAYER_H / 2.0f;

    for (auto& b : enemyBullets_) {
        if (!b.active) continue;
        if (b.pos.x >= px && b.pos.x <= px + SI_PLAYER_W &&
            b.pos.y >= py && b.pos.y <= py + SI_PLAYER_H) {
            b.active = false;
            --lives_;
            bus_.publish(Event::Score);
            state_ = (lives_ <= 0) ? SIState::Lost : SIState::Waiting;
            return;
        }
    }
}

bool SpaceInvadersGame::anyInvaderReachedPlayer() const
{
    for (int r = 0; r < SI_ROWS; ++r)
        for (int c = 0; c < SI_COLS; ++c)
            if (invaders_[r][c] &&
                swarmY_ + static_cast<float>(r) * SI_CELL_H + SI_INVADER_H >= SI_PLAYER_Y - SI_PLAYER_H)
                return true;
    return false;
}

int SpaceInvadersGame::leftmostAliveCol() const
{
    for (int c = 0; c < SI_COLS; ++c)
        for (int r = 0; r < SI_ROWS; ++r)
            if (invaders_[r][c]) return c;
    return -1;
}

int SpaceInvadersGame::rightmostAliveCol() const
{
    for (int c = SI_COLS - 1; c >= 0; --c)
        for (int r = 0; r < SI_ROWS; ++r)
            if (invaders_[r][c]) return c;
    return -1;
}

void SpaceInvadersGame::draw() const
{
    clearScreen();

    // Invaders
    for (int r = 0; r < SI_ROWS; ++r) {
        const Color& col = ROW_COLORS[r];
        SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 255);
        const int type = invaderType(r);
        for (int c = 0; c < SI_COLS; ++c) {
            if (!invaders_[r][c]) continue;
            const float cellX = swarmX_ + static_cast<float>(c) * SI_CELL_W;
            const float cellY = swarmY_ + static_cast<float>(r) * SI_CELL_H;
            drawInvader(renderer_, cellX + (SI_CELL_W - SI_INVADER_W) / 2.0f,
                                   cellY + (SI_CELL_H - SI_INVADER_H) / 2.0f, type);
        }
    }

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    drawHUD(renderer_, score_, lives_, SI_LIVES, level_);

    // Ground line
    SDL_FRect ground{0, SI_PLAYER_Y + SI_PLAYER_H / 2.0f + 8.0f, WINDOW_W, 2.0f};
    SDL_RenderFillRect(renderer_, &ground);

    // Player ship: flat body + gun barrel
    SDL_FRect ship{playerX_ - SI_PLAYER_W / 2.0f, SI_PLAYER_Y - SI_PLAYER_H / 2.0f,
                   SI_PLAYER_W, SI_PLAYER_H};
    SDL_FRect gun {playerX_ - 2.0f, SI_PLAYER_Y - SI_PLAYER_H / 2.0f - 7.0f, 4.0f, 7.0f};
    SDL_RenderFillRect(renderer_, &ship);
    SDL_RenderFillRect(renderer_, &gun);

    // Player bullet (yellow)
    if (playerBullet_.active) {
        SDL_SetRenderDrawColor(renderer_, 255, 220, 50, 255);
        SDL_FRect br{playerBullet_.pos.x - SI_PBULLET_W / 2.0f,
                     playerBullet_.pos.y,
                     SI_PBULLET_W, SI_PBULLET_H};
        SDL_RenderFillRect(renderer_, &br);
    }

    // Enemy bullets (red)
    SDL_SetRenderDrawColor(renderer_, 220, 60, 60, 255);
    for (const auto& b : enemyBullets_) {
        if (!b.active) continue;
        SDL_FRect br{b.pos.x - SI_EBULLET_W / 2.0f,
                     b.pos.y,
                     SI_EBULLET_W, SI_EBULLET_H};
        SDL_RenderFillRect(renderer_, &br);
    }

    // Overlays
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    if (state_ == SIState::Waiting) {
        constexpr float sc = 6.0f;
        const char* msg = "SPACE TO START";
        const float w = static_cast<float>(std::strlen(msg)) * 4.0f * sc;
        drawText(renderer_, msg, WINDOW_W / 2.0f - w / 2.0f, SI_PLAYER_Y - 120.0f, sc);
    } else if (state_ == SIState::Lost) {
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
