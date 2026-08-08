#pragma once
#include "constants.hpp"

constexpr int   SI_ROWS = 5;
constexpr int   SI_COLS = 11;
constexpr int   SI_LIVES = 3;
constexpr int   SI_MAX_ENEMY_BULLETS = 3;

constexpr float SI_CELL_W      = 44.0f;
constexpr float SI_CELL_H      = 36.0f;
constexpr float SI_INVADER_W   = 30.0f;
constexpr float SI_INVADER_H   = 22.0f;
constexpr float SI_WALL_MARGIN = 30.0f;
constexpr float SI_STEP_DOWN   = 20.0f;

constexpr float SI_SWARM_START_X = (WINDOW_W - SI_COLS * SI_CELL_W) / 2.0f;
constexpr float SI_SWARM_START_Y = 100.0f;

constexpr float SI_MARCH_BASE = 40.0f;
constexpr float SI_MARCH_INC  = 4.0f;   // per killed invader

constexpr float SI_PLAYER_W     = 40.0f;
constexpr float SI_PLAYER_H     = 16.0f;
constexpr float SI_PLAYER_Y     = WINDOW_H - 60.0f;
constexpr float SI_PLAYER_SPEED = 420.0f;

constexpr float SI_PBULLET_W     = 3.0f;
constexpr float SI_PBULLET_H     = 12.0f;
constexpr float SI_PBULLET_SPEED = 580.0f;

constexpr float SI_EBULLET_W     = 4.0f;
constexpr float SI_EBULLET_H     = 12.0f;
constexpr float SI_EBULLET_SPEED = 200.0f;

constexpr float SI_SHOOT_INTERVAL_BASE = 1.5f;
constexpr float SI_SHOOT_INTERVAL_MIN  = 0.3f;
constexpr float SI_SHOOT_INTERVAL_DEC  = 0.015f;  // per killed invader
constexpr float SI_SHOOT_INTERVAL_LDEC = 0.1f;    // per level

constexpr float SI_LEVEL_MARCH_INC = 20.0f;  // extra base march speed per level
