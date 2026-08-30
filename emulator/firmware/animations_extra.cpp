#include "animations_extra.h"

#include <math.h>

#include "animation_shared.h"

// ---------------------------------------------------------------- Campfire

class CampfireAnim : public Animation {
public:
    const char *name() const override { return "Campfire"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.fillScreen(COL_BLACK);
        drawNightSky(c, ms, 100);
        c.drawLine(0, 110, ANIM_W, 110, COL_DIMGRAY);

        drawSeated(c, 38, 108);
        drawSeated(c, 90, 108);

        float flicker = 0.7f + 0.3f * sinf(ms * 0.008f);
        int baseY = 108;
        int cx = 64;
        uint16_t flame = rgb565((uint8_t)(220 * flicker), (uint8_t)(120 * flicker), 20);
        uint16_t core = rgb565(255, (uint8_t)(200 * flicker), 40);

        for (int i = 0; i < 4; i++) {
            float j = frand(ms / 80 + i * 17) * 4 - 2;
            int h = (int)(14 * flicker + j);
            c.fillTriangle(cx - 8 + i * 2, baseY, cx + i * 2, baseY - h, cx + 8 + i * 2, baseY,
                           flame);
        }
        c.fillTriangle(cx - 4, baseY, cx, baseY - (int)(10 * flicker), cx + 4, baseY, core);

        for (int s = 0; s < 5; s++) {
            uint32_t seed = s * 97 + 3;
            float life = (ms * 0.001f + frand(seed) * 4.0f);
            life = fmodf(life, 4.0f);
            if (life > 3.2f) continue;
            int sx = cx + (int)((frand(seed + 1) - 0.5f) * 16);
            int sy = baseY - (int)(life * 28);
            uint16_t spark = life < 1.5f ? COL_YELLOW : COL_DIMGRAY;
            c.drawPixel(sx, sy, spark);
            if (life < 0.8f) c.drawPixel(sx + 1, sy - 1, COL_ORANGE);
        }
    }
};

// ---------------------------------------------------------------- Owl

class OwlAnim : public Animation {
public:
    const char *name() const override { return "Owl"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.fillScreen(COL_BLACK);
        c.drawLine(20, 72, 108, 68, COL_WOOD);
        c.drawLine(20, 73, 108, 69, COL_DIMGRAY);

        int ox = 64, oy = 58;
        c.fillCircle(ox, oy, 14, COL_GRAY);
        c.fillTriangle(ox - 10, oy - 10, ox - 6, oy - 18, ox - 2, oy - 10, COL_GRAY);
        c.fillTriangle(ox + 2, oy - 10, ox + 6, oy - 18, ox + 10, oy - 10, COL_GRAY);

        constexpr uint32_t CYCLE = 6000;
        uint32_t t = ms % CYCLE;
        float headAngle = 0;
        if (t < 2000)
            headAngle = -1.0f + t / 1000.0f;
        else if (t < 4000)
            headAngle = 1.0f - (t - 2000) / 1000.0f;

        bool blink = t > 2400 && t < 2600;
        int pupilOff = (int)(headAngle * 2);

        c.fillCircle(ox - 5, oy - 2, 4, COL_WHITE);
        c.fillCircle(ox + 5, oy - 2, 4, COL_WHITE);
        if (!blink) {
            c.fillCircle(ox - 5 + pupilOff, oy - 2, 2, COL_BLACK);
            c.fillCircle(ox + 5 + pupilOff, oy - 2, 2, COL_BLACK);
        } else {
            c.drawLine(ox - 8, oy - 2, ox - 2, oy - 2, COL_BLACK);
            c.drawLine(ox + 2, oy - 2, ox + 8, oy - 2, COL_BLACK);
        }
        c.fillCircle(ox, oy + 4, 3, COL_ORANGE);

        if (t >= 3000) {
            float mp = (t - 3000) / 2000.0f;
            if (mp <= 1.0f) {
                int mx = (int)(mp * (ANIM_W - 16)) + 4;
                int my = ANIM_H - 6;
                c.fillRect(mx, my, 5, 2, COL_GRAY);
                c.drawPixel(mx + 5, my, COL_WHITE);
                c.drawPixel(mx - 1, my + 1, COL_GRAY);
            }
        }
    }
};

// ---------------------------------------------------------------- Juggler

class JugglerAnim : public Animation {
public:
    const char *name() const override { return "Juggler"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.fillScreen(COL_BLACK);
        c.drawLine(10, 115, ANIM_W - 10, 115, COL_DIMGRAY);

        int cx = 64 + (int)(sinf(ms * 0.005f) * 2);
        drawJuggler(c, cx, 100, ms);
    }
};

static CampfireAnim campfire;
static OwlAnim owl;
static JugglerAnim juggler;

static Animation *extraList[] = {&campfire, &owl, &juggler};

Animation **extraAnimationList(int &count) {
    count = 3;
    return extraList;
}
