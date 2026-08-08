#include "renderer_utils.hpp"
#include "constants.hpp"
#include <cstdint>
#include <string>

namespace {

constexpr std::uint8_t DIGITS[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111},  // 0
    {0b010, 0b110, 0b010, 0b010, 0b111},  // 1
    {0b111, 0b001, 0b111, 0b100, 0b111},  // 2
    {0b111, 0b001, 0b111, 0b001, 0b111},  // 3
    {0b101, 0b101, 0b111, 0b001, 0b001},  // 4
    {0b111, 0b100, 0b111, 0b001, 0b111},  // 5
    {0b111, 0b100, 0b111, 0b101, 0b111},  // 6
    {0b111, 0b001, 0b010, 0b010, 0b010},  // 7
    {0b111, 0b101, 0b111, 0b101, 0b111},  // 8
    {0b111, 0b101, 0b111, 0b001, 0b111},  // 9
};

constexpr std::uint8_t LETTERS[26][5] = {
    {0b111, 0b101, 0b111, 0b101, 0b101},  // A
    {0b110, 0b101, 0b110, 0b101, 0b110},  // B
    {0b111, 0b100, 0b100, 0b100, 0b111},  // C
    {0b110, 0b101, 0b101, 0b101, 0b110},  // D
    {0b111, 0b100, 0b110, 0b100, 0b111},  // E
    {0b111, 0b100, 0b110, 0b100, 0b100},  // F
    {0b111, 0b100, 0b101, 0b101, 0b111},  // G
    {0b101, 0b101, 0b111, 0b101, 0b101},  // H
    {0b111, 0b010, 0b010, 0b010, 0b111},  // I
    {0b001, 0b001, 0b001, 0b101, 0b111},  // J
    {0b101, 0b110, 0b110, 0b101, 0b101},  // K
    {0b100, 0b100, 0b100, 0b100, 0b111},  // L
    {0b111, 0b111, 0b101, 0b101, 0b101},  // M
    {0b110, 0b101, 0b101, 0b101, 0b101},  // N
    {0b111, 0b101, 0b101, 0b101, 0b111},  // O
    {0b111, 0b101, 0b111, 0b100, 0b100},  // P
    {0b111, 0b101, 0b101, 0b111, 0b001},  // Q
    {0b110, 0b101, 0b110, 0b101, 0b101},  // R
    {0b111, 0b100, 0b111, 0b001, 0b111},  // S
    {0b111, 0b010, 0b010, 0b010, 0b010},  // T
    {0b101, 0b101, 0b101, 0b101, 0b111},  // U
    {0b101, 0b101, 0b101, 0b010, 0b010},  // V
    {0b101, 0b101, 0b101, 0b111, 0b111},  // W
    {0b101, 0b101, 0b010, 0b101, 0b101},  // X
    {0b101, 0b101, 0b111, 0b010, 0b010},  // Y
    {0b111, 0b001, 0b010, 0b100, 0b111},  // Z
};

void drawGlyph(SDL_Renderer* r, const std::uint8_t rows[5], float x, float y, float scale)
{
    SDL_FRect cell{0, 0, scale, scale};
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (rows[row] & (0b100 >> col)) {
                cell.x = x + static_cast<float>(col) * scale;
                cell.y = y + static_cast<float>(row) * scale;
                SDL_RenderFillRect(r, &cell);
            }
        }
    }
}

// Symbols indexed by: '!' 33, '"' 34 ... up to '/' 47, then ':' 58...
// We only need a handful; store as {char, glyph} pairs.
struct Symbol { char ch; std::uint8_t rows[5]; };
constexpr Symbol SYMBOLS[] = {
    {'>', {0b100, 0b010, 0b001, 0b010, 0b100}},
    {'<', {0b001, 0b010, 0b100, 0b010, 0b001}},
    {'-', {0b000, 0b000, 0b111, 0b000, 0b000}},
    {'!', {0b010, 0b010, 0b010, 0b000, 0b010}},
};

} // namespace

void drawText(SDL_Renderer* r, const char* text, float x, float y, float scale)
{
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            x += 4.0f * scale;
        } else if (*p >= '0' && *p <= '9') {
            drawGlyph(r, DIGITS[*p - '0'], x, y, scale);
            x += 4.0f * scale;
        } else if (*p >= 'A' && *p <= 'Z') {
            drawGlyph(r, LETTERS[*p - 'A'], x, y, scale);
            x += 4.0f * scale;
        } else {
            for (const auto& sym : SYMBOLS) {
                if (sym.ch == *p) {
                    drawGlyph(r, sym.rows, x, y, scale);
                    break;
                }
            }
            x += 4.0f * scale;
        }
    }
}

void drawNumber(SDL_Renderer* r, int number, float x, float y, float scale)
{
    const std::string text = std::to_string(number);
    for (char c : text) {
        drawGlyph(r, DIGITS[c - '0'], x, y, scale);
        x += 4.0f * scale;
    }
}

void drawCenterLine(SDL_Renderer* r)
{
    constexpr float dashH = 18.0f;
    constexpr float gap   = 14.0f;
    SDL_FRect dash{WINDOW_W / 2.0f - 3.0f, 0, 6.0f, dashH};
    for (float y = 0.0f; y < WINDOW_H; y += dashH + gap) {
        dash.y = y;
        SDL_RenderFillRect(r, &dash);
    }
}
