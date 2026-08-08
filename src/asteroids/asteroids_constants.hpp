#pragma once
#include "constants.hpp"

constexpr float AST_SHIP_RADIUS     = 12.0f;
constexpr float AST_SHIP_ACCEL      = 280.0f;
constexpr float AST_SHIP_ROTATE     = 3.2f;    // rad/s
constexpr float AST_SHIP_DRAG       = 0.35f;   // velocity loss fraction per second

constexpr int   AST_MAX_BULLETS     = 4;
constexpr float AST_BULLET_SPEED    = 520.0f;
constexpr float AST_BULLET_LIFE     = 1.3f;

constexpr float AST_RAD_LARGE       = 48.0f;
constexpr float AST_RAD_MEDIUM      = 26.0f;
constexpr float AST_RAD_SMALL       = 13.0f;

constexpr float AST_SPEED_LARGE     = 55.0f;
constexpr float AST_SPEED_MEDIUM    = 100.0f;
constexpr float AST_SPEED_SMALL     = 160.0f;
constexpr float AST_LEVEL_SPEED_INC = 8.0f;    // per level

constexpr int   AST_VERTS           = 10;
constexpr int   AST_LIVES           = 3;
constexpr int   AST_START_LARGE     = 3;        // level 1 starting count
constexpr int   AST_MAX_LARGE       = 8;
constexpr float AST_SPAWN_SAFE_DIST = 140.0f;
constexpr float AST_INVINCIBLE_TIME = 2.5f;

constexpr int   AST_SCORE_LARGE     = 20;
constexpr int   AST_SCORE_MEDIUM    = 50;
constexpr int   AST_SCORE_SMALL     = 100;
