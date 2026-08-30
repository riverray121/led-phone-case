#pragma once

#include <stdint.h>

#include "Adafruit_GFX.h"

constexpr int ANIM_W = 128;
constexpr int ANIM_H = 128;
constexpr float ANIM_PI = 3.14159265f;

constexpr uint16_t COL_BLACK = 0x0000;
constexpr uint16_t COL_WHITE = 0xFFFF;
constexpr uint16_t COL_GRAY = 0x8410;
constexpr uint16_t COL_DIMGRAY = 0x39E7;
constexpr uint16_t COL_RED = 0xF800;
constexpr uint16_t COL_YELLOW = 0xFFE0;
constexpr uint16_t COL_MOON = 0xFFF2;
constexpr uint16_t COL_WATER = 0x0119;
constexpr uint16_t COL_WAVE = 0x1C9F;
constexpr uint16_t COL_WOOD = 0xA285;
constexpr uint16_t COL_HILL = 0x01E2;
constexpr uint16_t COL_SKY_STAR = 0x7BEF;
constexpr uint16_t COL_ORANGE = 0xFD20;

inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint32_t hash32(uint32_t x);
float frand(uint32_t seed);

struct Frame2D {
    float ox, oy, dx, dy, ux, uy;

    int X(float a, float h) const;
    int Y(float a, float h) const;

    static Frame2D upright(float x, float y);
    void flip();
};

struct EdgePath {
    static constexpr float MARGIN = 2;
    static constexpr float R = 16;
    static constexpr float SL = ANIM_W - 2 * MARGIN - 2 * R;
    static constexpr float AL = R * ANIM_PI / 2;

    static float total();
    static Frame2D at(float s);
};

enum class Pose { Run, Stand, Look, ArmsUp, Jump, Slump, Push };

void drawHuman(GFXcanvas16 &c, const Frame2D &f, Pose pose, float phase, uint32_t ms,
               uint16_t col);
void drawSeated(GFXcanvas16 &c, int x, int y, uint16_t col = COL_WHITE);
void drawNightSky(GFXcanvas16 &c, uint32_t ms, int skyBottomY);

void juggleBallPos(uint32_t ms, int ball, int &x, int &y);
void drawJuggler(GFXcanvas16 &c, int cx, int cy, uint32_t ms);
