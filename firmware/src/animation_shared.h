// Helpers shared by the scene animations: canvas constants, palette, hashing,
// the local drawing frame, the screen-edge path, and stick-figure drawing.
// Compiled into the firmware and into the browser emulator, so it must not
// depend on Arduino or hardware headers beyond Adafruit_GFX.
#pragma once

#include <stdint.h>

#include <Adafruit_GFX.h>

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

// Figures are drawn in a local frame: a = forward along travel, h = up away
// from the ground, so the same body code works on any edge or slope.
struct Frame2D {
    float ox, oy, dx, dy, ux, uy;

    int X(float a, float h) const;
    int Y(float a, float h) const;

    static Frame2D upright(float x, float y);
    void flip();
};

// Perimeter path hugging the screen edge, with quarter-circle corners so a
// walker's orientation turns smoothly instead of snapping 90 degrees.
struct EdgePath {
    static constexpr float MARGIN = 2;
    static constexpr float R = 16;
    static constexpr float SL = ANIM_W - 2 * MARGIN - 2 * R;  // straight side length
    static constexpr float AL = R * ANIM_PI / 2;              // corner arc length

    static float total();
    static Frame2D at(float s);
};

enum class Pose { Run, Stand, Look, ArmsUp, Jump, Slump, Push };

// Articulated stick figure. Two-segment limbs: thighs and shins with a knee,
// upper arms and forearms with an elbow. Roughly 23 px tall.
void drawHuman(GFXcanvas16 &c, const Frame2D &f, Pose pose, float phase, uint32_t ms,
               uint16_t col);

// Seated figure leaning back on their arms, face tilted up at the sky.
void drawSeated(GFXcanvas16 &c, int x, int y, uint16_t col = COL_WHITE);

// Twinkling star field and moon, filling the screen down to skyBottomY.
void drawNightSky(GFXcanvas16 &c, uint32_t ms, int skyBottomY);
