#pragma once
#include <SDL3/SDL.h>

void drawText(SDL_Renderer* r, const char* text, float x, float y, float scale);
void drawNumber(SDL_Renderer* r, int number, float x, float y, float scale);
void drawCenterLine(SDL_Renderer* r);
void drawHUD(SDL_Renderer* r, int score, int lives, int maxLives, int level);
