#include "animations.h"

#include <math.h>

#include "animation_shared.h"

namespace {

// ---------------------------------------------------------------- Face

class FaceAnim : public Animation {
public:
    const char *name() const override { return "Face"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        uint32_t seg = ms / 2000;
        uint32_t segMs = ms % 2000;
        float t = segMs < 350 ? segMs / 350.0f : 1.0f;
        t = t * t * (3 - 2 * t);
        float px = lookX(seg - 1) + (lookX(seg) - lookX(seg - 1)) * t;
        float py = lookY(seg - 1) + (lookY(seg) - lookY(seg - 1)) * t;

        bool blink = (ms % 3700) < 140;

        for (int ex : {40, 88}) {
            if (blink) {
                c.fillRect(ex - 20, 48, 41, 4, COL_WHITE);
            } else {
                c.fillCircle(ex, 50, 20, COL_WHITE);
                c.fillCircle(ex + (int)(px * 9), 50 + (int)(py * 8), 8, COL_BLACK);
            }
        }

        if (seg % 6 == 4) {
            c.fillCircle(64, 100, 9, COL_WHITE);
            c.fillCircle(64, 100, 5, COL_BLACK);
        } else {
            for (int x = -22; x <= 22; x += 2) {
                int y = 96 + (int)(10 - (x * x) / 48.0f);
                c.fillRect(64 + x, y, 3, 3, COL_WHITE);
            }
        }
    }

private:
    float lookX(uint32_t seg) { return frand(seg * 2 + 11) * 2 - 1; }
    float lookY(uint32_t seg) { return frand(seg * 2 + 12) * 2 - 1; }
};

// ---------------------------------------------------------------- Fisherman

class FishermanAnim : public Animation {
public:
    const char *name() const override { return "Fisherman"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.fillCircle(100, 22, 10, COL_MOON);
        c.fillCircle(105, 19, 9, COL_BLACK);
        for (int i = 0; i < 9; i++) {
            int sx = (int)(frand(i * 7 + 1) * 120) + 4;
            int sy = (int)(frand(i * 7 + 2) * 60) + 4;
            uint16_t col = (ms / 400 + i) % 4 ? COL_SKY_STAR : COL_WHITE;
            c.drawPixel(sx, sy, col);
        }

        c.fillRect(0, 86, ANIM_W, ANIM_H - 86, COL_WATER);
        for (int row = 0; row < 4; row++) {
            int baseY = 90 + row * 9;
            for (int x = 0; x < ANIM_W; x += 3) {
                float ph = ms * 0.002f + x * 0.12f + row * 1.7f;
                c.drawPixel(x, baseY + (int)(sinf(ph) * 2), COL_WAVE);
            }
        }

        int xb = -60 + (int)((ms / 55) % 250);
        int bob = (int)(sinf(ms * 0.003f) * 1.5f);
        int yb = 84 + bob;

        c.fillTriangle(xb, yb, xb + 8, yb + 6, xb + 44, yb, COL_WOOD);
        c.fillTriangle(xb + 8, yb + 6, xb + 36, yb + 6, xb + 44, yb, COL_WOOD);

        int xf = xb + 36;
        int fy = yb;
        c.drawLine(xf - 2, fy, xf, fy - 7, COL_WHITE);
        c.drawLine(xf + 2, fy, xf, fy - 7, COL_WHITE);
        c.drawLine(xf, fy - 7, xf - 1, fy - 16, COL_WHITE);
        c.fillCircle(xf - 1, fy - 19, 2, COL_WHITE);
        c.fillTriangle(xf - 6, fy - 21, xf + 4, fy - 21, xf - 1, fy - 25, COL_YELLOW);

        float stroke = sinf(ms * 0.004f) * 0.5f + 0.25f;
        int hx = xf - 3, hy = fy - 13;
        int px2 = hx - (int)(sinf(stroke) * 26);
        int py2 = hy + (int)(cosf(stroke) * 26);
        c.drawLine(hx, hy - 3, px2, py2, COL_WOOD);
        c.drawLine(xf + 1, fy - 12, hx, hy - 3, COL_WHITE);
    }
};

// ---------------------------------------------------------------- Runner

class RunnerAnim : public Animation {
public:
    const char *name() const override { return "Runner"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        // 4.2s blocks: run 110 px, then either pause and look around or keep
        // going. Distance accumulates so position is always continuous.
        constexpr uint32_t BLOCK = 4200;
        uint32_t block = ms / BLOCK;
        uint32_t bp = ms % BLOCK;

        float dist = 0;
        for (uint32_t b = 0; b < block; b++) dist += pauses(b) ? 110.f : 154.f;

        bool running = true;
        uint32_t lookMs = 0;
        if (bp < 3000) {
            dist += 110.f * bp / 3000.f;
        } else if (pauses(block)) {
            dist += 110.f;
            running = false;
            lookMs = bp - 3000;
        } else {
            dist += 110.f + 44.f * (bp - 3000) / 1200.f;
        }

        Frame2D f = EdgePath::at(dist);
        if (running) {
            drawHuman(c, f, Pose::Run, dist * 0.55f, ms, COL_WHITE);
        } else {
            drawHuman(c, f, Pose::Look, 0, ms, COL_WHITE);
            if ((lookMs / 400) % 2) {
                c.setTextColor(COL_YELLOW);
                c.setCursor(f.X(0, 29) - 2, f.Y(0, 29) - 3);
                c.print('?');
            }
        }
    }

private:
    static bool pauses(uint32_t block) { return frand(block * 7 + 31) < 0.4f; }
};

// ---------------------------------------------------------------- Sisyphus

class SisyphusAnim : public Animation {
public:
    const char *name() const override { return "Sisyphus"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        // each 6.6s cycle nets 100 px of progress: push to 75, the boulder
        // slips back to 45 while he watches, he trudges back, pushes on to 100
        constexpr float BR = 12;  // boulder radius
        constexpr float GAP = BR + 5;  // figure trails the boulder center
        uint32_t cycle = ms / 6600;
        uint32_t p = ms % 6600;
        float base = fmodf(cycle * 100.0f, EdgePath::total());

        float uf, ub;
        enum { PUSHING, WATCHING, RETURNING } state = PUSHING;
        if (p < 3000) {
            uf = ub = 75.0f * p / 3000.0f;
        } else if (p < 3600) {
            uf = 75;
            ub = 75 - 30.0f * (p - 3000) / 600.0f;
            state = WATCHING;
        } else if (p < 4400) {
            uf = 75 - 30.0f * (p - 3600) / 800.0f;
            ub = 45;
            state = RETURNING;
        } else {
            uf = ub = 45 + 55.0f * (p - 4400) / 2200.0f;
        }

        // boulder, rolling: spoke angle is arc length over radius
        Frame2D fb = EdgePath::at(base + ub);
        int bx = fb.X(0, BR), by = fb.Y(0, BR);
        float rot = (base + ub) / BR;
        c.fillCircle(bx, by, (int)BR, COL_GRAY);
        for (int k = 0; k < 3; k++) {
            float a = rot + k * 2.094f;
            c.drawLine(bx, by, bx + (int)(cosf(a) * (BR - 3)),
                       by + (int)(sinf(a) * (BR - 3)), COL_DIMGRAY);
        }

        Frame2D ff = EdgePath::at(base + uf - GAP);
        switch (state) {
            case PUSHING:
                drawHuman(c, ff, Pose::Push, (base + uf) * 0.5f, ms, COL_WHITE);
                break;
            case WATCHING:
                drawHuman(c, ff, Pose::Slump, 0, ms, COL_WHITE);
                break;
            case RETURNING:
                ff.flip();  // he walks facing the way he trudges
                drawHuman(c, ff, Pose::Run, (base + uf) * 0.4f, ms, COL_WHITE);
                break;
        }
    }
};

// ---------------------------------------------------------------- Balloon

class BalloonAnim : public Animation {
public:
    const char *name() const override { return "Balloon"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        int bx = -20 + (int)((ms / 45) % 190);
        int by = 36 + (int)(sinf(ms * 0.002f) * 9);

        c.fillCircle(bx, by, 9, COL_RED);
        c.fillTriangle(bx - 3, by + 9, bx + 3, by + 9, bx, by + 12, COL_RED);
        for (int i = 0; i < 16; i++) {
            c.drawPixel(bx + (int)(sinf(ms * 0.004f + i * 0.6f) * 2),
                        by + 12 + i, COL_WHITE);
        }

        c.drawLine(0, 105, ANIM_W, 105, COL_DIMGRAY);

        // the kid sprints after it and leaps mid-screen, never catching it
        float feetY = 104;
        Pose pose = Pose::ArmsUp;
        if (bx > 55 && bx < 90) {
            feetY -= sinf((bx - 55) * ANIM_PI / 35.0f) * 13;
            pose = Pose::Jump;
        }
        Frame2D f = Frame2D::upright(bx - 30, feetY);
        drawHuman(c, f, pose, ms * 0.014f, ms, COL_WHITE);
    }
};

// ---------------------------------------------------------------- Stargazer

class StargazerAnim : public Animation {
public:
    const char *name() const override { return "Stargazer"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        drawNightSky(c, ms, 98);

        uint32_t sp = ms % 7000;
        if (sp < 700) {
            uint32_t ep = ms / 7000;
            int ox = (int)(frand(ep + 21) * 70) + 10;
            int oy = (int)(frand(ep + 22) * 30) + 8;
            int d = sp / 14;
            c.drawLine(ox + d - 8, oy + d / 2 - 4, ox + d, oy + d / 2, COL_WHITE);
        }

        c.fillCircle(64, 176, 74, COL_HILL);
        drawSeated(c, 46, 104);
        drawSeated(c, 74, 104);
    }
};

// ---------------------------------------------------------------- Campfire

class CampfireAnim : public Animation {
public:
    const char *name() const override { return "Campfire"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
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

        // sparks drift up and fade from yellow through gray
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
        c.drawLine(20, 72, 108, 68, COL_WOOD);
        c.drawLine(20, 73, 108, 69, COL_DIMGRAY);

        int ox = 64, oy = 58;
        c.fillCircle(ox, oy, 14, COL_GRAY);
        c.fillTriangle(ox - 10, oy - 10, ox - 6, oy - 18, ox - 2, oy - 10, COL_GRAY);
        c.fillTriangle(ox + 2, oy - 10, ox + 6, oy - 18, ox + 10, oy - 10, COL_GRAY);

        // head sweeps left to right and back, blinking at the turn
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

        // a mouse scurries across the ground while the owl watches
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

// Balls follow a three-ball cascade: each flies hand to hand on a sine arc,
// staggered a third of a period apart.
void juggleBallPos(uint32_t ms, int ball, int &x, int &y) {
    constexpr float PERIOD = 1100.0f;
    constexpr float LEFT_X = 48.0f;
    constexpr float RIGHT_X = 80.0f;
    constexpr float HAND_Y = 90.0f;
    constexpr float APEX_Y = 38.0f;

    float t = fmodf(ms / PERIOD + ball / 3.0f, 1.0f);
    float half = t * 2.0f;
    int segment = (int)half;
    float local = half - segment;
    float x0 = (segment % 2 == 0) ? LEFT_X : RIGHT_X;
    float x1 = (segment % 2 == 0) ? RIGHT_X : LEFT_X;
    x = (int)(x0 + (x1 - x0) * local + 0.5f);
    y = (int)(HAND_Y - sinf(local * ANIM_PI) * (HAND_Y - APEX_Y) + 0.5f);
}

// two-segment arm bent slightly at the elbow, reaching toward a target
void drawArmTo(GFXcanvas16 &c, int sx, int sy, int tx, int ty, uint16_t col) {
    int mx = (sx + tx) / 2;
    int my = (sy + ty) / 2 - 2;
    c.drawLine(sx, sy, mx, my, col);
    c.drawLine(mx, my, tx, ty, col);
}

class JugglerAnim : public Animation {
public:
    const char *name() const override { return "Juggler"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.drawLine(10, 115, ANIM_W - 10, 115, COL_DIMGRAY);

        int cx = 64 + (int)(sinf(ms * 0.005f) * 2);
        int bx[3], by[3];
        for (int i = 0; i < 3; i++) juggleBallPos(ms, i, bx[i], by[i]);

        Frame2D f = Frame2D::upright((float)cx, 100.0f);
        float lean = 1.0f;
        float shoulderH = 16.0f;

        auto leg = [&](float thigh, float bend) {
            float ka = sinf(thigh) * 5.5f, kh = 9 - cosf(thigh) * 5.5f;
            float shin = thigh - bend;
            float fa = ka + sinf(shin) * 5.0f, fh = kh - cosf(shin) * 5.0f;
            if (fh < -0.5f) fh = -0.5f;
            c.drawLine(f.X(0, 9), f.Y(0, 9), f.X(ka, kh), f.Y(ka, kh), COL_WHITE);
            c.drawLine(f.X(ka, kh), f.Y(ka, kh), f.X(fa, fh), f.Y(fa, fh), COL_WHITE);
        };
        leg(0.12f, 0.1f);
        leg(-0.12f, 0.08f);

        c.drawLine(f.X(0, 9), f.Y(0, 9), f.X(lean, shoulderH), f.Y(lean, shoulderH),
                   COL_WHITE);
        c.fillCircle(f.X(0, 20), f.Y(0, 20), 3, COL_WHITE);

        // each hand tracks whichever ball is nearest its side
        int lsx = f.X(lean, shoulderH);
        int lsy = f.Y(lean, shoulderH);
        int leftTarget = 0, rightTarget = 0;
        float leftDist = 999.0f, rightDist = 999.0f;
        for (int i = 0; i < 3; i++) {
            float dl = fabsf((float)bx[i] - 48.0f);
            float dr = fabsf((float)bx[i] - 80.0f);
            if (dl < leftDist) { leftDist = dl; leftTarget = i; }
            if (dr < rightDist) { rightDist = dr; rightTarget = i; }
        }
        drawArmTo(c, lsx, lsy, bx[leftTarget], by[leftTarget], COL_WHITE);
        drawArmTo(c, lsx, lsy, bx[rightTarget], by[rightTarget], COL_WHITE);

        const uint16_t colors[3] = {COL_RED, COL_YELLOW, COL_WHITE};
        for (int i = 0; i < 3; i++) c.fillCircle(bx[i], by[i], 4, colors[i]);
    }
};

// ------------------------------------------------------------ low-res set
// Drawn on an 8x8 logical grid of 16x16 blocks, sized for the LED matrix.
// The matrix driver's block averaging reproduces them one LED per cell; on
// the TFT they render as chunky pixel art.

constexpr int CELL = ANIM_W / 8;

void px(GFXcanvas16 &c, int x, int y, uint16_t col) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;
    c.fillRect(x * CELL, y * CELL, CELL, CELL, col);
}

uint16_t hueColor(uint8_t h) {
    uint8_t x = (h % 85) * 3;
    if (h < 85) return rgb565(255 - x, x, 0);
    if (h < 170) return rgb565(0, 255 - x, x);
    return rgb565(x, 0, 255 - x);
}

class RainbowAnim : public Animation {
public:
    const char *name() const override { return "Rainbow"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                px(c, x, y, hueColor((uint8_t)((x + y) * 14 + ms / 14)));
    }
};

class FireAnim : public Animation {
public:
    const char *name() const override { return "Fire"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        frameNo_++;
        // classic fire: bottom row sparks, heat rises and cools
        for (int x = 0; x < 8; x++)
            heat_[7 * 8 + x] = 140 + (hash32(frameNo_ * 8 + x) % 116);
        for (int y = 0; y < 7; y++) {
            for (int x = 0; x < 8; x++) {
                int xl = x > 0 ? x - 1 : 0, xr = x < 7 ? x + 1 : 7;
                int below = y + 1;
                int h = (heat_[below * 8 + x] * 2 + heat_[below * 8 + xl] +
                         heat_[below * 8 + xr]) / 4;
                int cool = 10 + (hash32(frameNo_ * 64 + y * 8 + x) % 22);
                heat_[y * 8 + x] = h > cool ? h - cool : 0;
            }
        }
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int t = heat_[y * 8 + x];
                uint8_t r = t < 85 ? t * 3 : 255;
                uint8_t g = t < 85 ? 0 : (t < 170 ? (t - 85) * 3 : 255);
                uint8_t b = t < 170 ? 0 : (t - 170) * 3;
                px(c, x, y, rgb565(r, g, b));
            }
        }
    }

private:
    uint8_t heat_[64] = {0};
    uint32_t frameNo_ = 0;
};

class RainAnim : public Animation {
public:
    const char *name() const override { return "Rain"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        for (int x = 0; x < 8; x++) {
            uint32_t speed = 120 + (uint32_t)(frand(x * 3 + 7) * 160);  // ms per cell
            uint32_t phase = (uint32_t)(frand(x * 5 + 2) * 4000);
            int head = (int)(((ms + phase) / speed) % 14);  // 8 rows + off-screen gap
            for (int t = 0; t < 4; t++) {
                int y = head - t;
                if (y < 0 || y > 7) continue;
                if (t == 0) px(c, x, y, rgb565(170, 220, 255));
                else px(c, x, y, rgb565(0, 40 / t, 200 / t));
            }
        }
    }
};

class HeartAnim : public Animation {
public:
    const char *name() const override { return "Heart"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        static const uint8_t rows[8] = {0b01100110, 0b11111111, 0b11111111,
                                        0b11111111, 0b01111110, 0b00111100,
                                        0b00011000, 0b00000000};
        // lub-dub: two quick pulses, then rest
        float t = ms % 1100;
        auto hump = [&](float center, float w) {
            float d = fabsf(t - center);
            return d < w ? 1.0f - d / w : 0.0f;
        };
        float amp = 0.30f + 0.70f * fmaxf(hump(150, 130), 0.75f * hump(430, 150));
        uint8_t r = (uint8_t)(70 + 185 * amp);
        uint16_t col = rgb565(r, 0, r / 5);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                if (rows[y] & (0x80 >> x)) px(c, x, y, col);
    }
};

class SnakeAnim : public Animation {
public:
    const char *name() const override { return "Snake"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        if (ms < lastMs_) inited_ = false;  // animation was reselected
        lastMs_ = ms;
        if (!inited_) reset(ms);

        if (ms - lastStep_ >= 170) {
            lastStep_ = ms;
            step();
        }

        px(c, foodX_, foodY_, COL_RED);
        for (int i = len_ - 1; i >= 0; i--) {
            uint8_t g = i == 0 ? 255 : (uint8_t)(190 - i * 6);
            px(c, bodyX_[i], bodyY_[i], rgb565(0, g, i == 0 ? 60 : 0));
        }
    }

private:
    static constexpr int MAX_LEN = 20;
    int8_t bodyX_[MAX_LEN], bodyY_[MAX_LEN];
    int len_ = 0;
    int8_t foodX_ = 5, foodY_ = 5;
    uint32_t lastStep_ = 0, lastMs_ = 0, seed_ = 0;
    bool inited_ = false;

    void reset(uint32_t ms) {
        inited_ = true;
        len_ = 3;
        for (int i = 0; i < 3; i++) { bodyX_[i] = 3 - i; bodyY_[i] = 4; }
        lastStep_ = ms;
        seed_ = hash32(ms);
        placeFood();
    }

    bool onBody(int x, int y, int upto) const {
        for (int i = 0; i < upto; i++)
            if (bodyX_[i] == x && bodyY_[i] == y) return true;
        return false;
    }

    void placeFood() {
        do {
            foodX_ = hash32(seed_++) % 8;
            foodY_ = hash32(seed_++) % 8;
        } while (onBody(foodX_, foodY_, len_));
    }

    void step() {
        int hx = bodyX_[0], hy = bodyY_[0];
        int dx = foodX_ - hx, dy = foodY_ - hy;
        // candidate moves: toward food on the longer axis first
        int cand[4][2];
        int px1 = dx > 0 ? 1 : -1, py1 = dy > 0 ? 1 : -1;
        if (abs(dx) >= abs(dy)) {
            cand[0][0] = px1; cand[0][1] = 0;
            cand[1][0] = 0;   cand[1][1] = py1;
            cand[2][0] = 0;   cand[2][1] = -py1;
            cand[3][0] = -px1; cand[3][1] = 0;
        } else {
            cand[0][0] = 0;   cand[0][1] = py1;
            cand[1][0] = px1; cand[1][1] = 0;
            cand[2][0] = -px1; cand[2][1] = 0;
            cand[3][0] = 0;   cand[3][1] = -py1;
        }
        int nx = -1, ny = -1;
        for (auto &m : cand) {
            int tx = hx + m[0], ty = hy + m[1];
            // tail cell is fine: it moves away this step
            if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8 && !onBody(tx, ty, len_ - 1)) {
                nx = tx; ny = ty;
                break;
            }
        }
        if (nx < 0) {  // trapped: start over
            inited_ = false;
            return;
        }

        bool ate = nx == foodX_ && ny == foodY_;
        int newLen = ate ? len_ + 1 : len_;
        if (newLen > MAX_LEN) {
            inited_ = false;
            return;
        }
        for (int i = newLen - 1; i > 0; i--) {
            bodyX_[i] = bodyX_[i - 1];
            bodyY_[i] = bodyY_[i - 1];
        }
        bodyX_[0] = nx;
        bodyY_[0] = ny;
        len_ = newLen;
        if (ate) placeFood();
    }
};

class SmileyAnim : public Animation {
public:
    const char *name() const override { return "Smiley"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        uint32_t seg = ms / 2200;
        float roll = frand(seg * 9 + 3);
        // 0 neutral (eyes wander), 1 smile, 2 surprised, 3 wink, 4 sad
        int expr = roll < 0.35f ? 0 : roll < 0.65f ? 1 : roll < 0.80f ? 2
                   : roll < 0.90f ? 3 : 4;

        uint16_t eye = rgb565(80, 200, 255);
        uint16_t mouth = rgb565(255, 190, 40);

        int look = expr == 0 ? (int)(frand(seg * 11 + 6) * 3) - 1 : 0;
        bool blink = (ms % 3400) < 150 && expr != 3;

        // eyes: 2x2 blocks, one row higher when surprised, a line when closed
        int eyeTop = expr == 2 ? 0 : 1;
        for (int ex : {1, 5}) {
            bool closed = blink || (expr == 3 && ex == 1);
            if (closed) {
                px(c, ex + look, 2, eye);
                px(c, ex + 1 + look, 2, eye);
            } else {
                for (int y = eyeTop; y <= 2; y++) {
                    px(c, ex + look, y, eye);
                    px(c, ex + 1 + look, y, eye);
                }
            }
        }

        switch (expr) {
            case 2:  // surprised: open mouth
                px(c, 3, 4, mouth); px(c, 4, 4, mouth);
                px(c, 3, 5, mouth); px(c, 4, 5, mouth);
                break;
            case 4:  // sad: corners down
                for (int x = 2; x <= 5; x++) px(c, x, 4, mouth);
                px(c, 1, 5, mouth); px(c, 6, 5, mouth);
                break;
            case 0:  // neutral: straight line
                for (int x = 2; x <= 5; x++) px(c, x, 5, mouth);
                break;
            default:  // smile (also under the wink)
                px(c, 1, 4, mouth); px(c, 6, 4, mouth);
                for (int x = 2; x <= 5; x++) px(c, x, 5, mouth);
                break;
        }
    }
};

FaceAnim face;
FishermanAnim fisherman;
RunnerAnim runner;
SisyphusAnim sisyphus;
BalloonAnim balloon;
StargazerAnim stargazer;
CampfireAnim campfire;
OwlAnim owl;
JugglerAnim juggler;
RainbowAnim rainbow;
FireAnim fire;
RainAnim rain;
HeartAnim heart;
SnakeAnim snake;
SmileyAnim smiley;

// Scene animations first, then the 8x8-native low-res set. sceneAnimationList
// exposes the scene prefix, so scenes must stay contiguous at the front.
Animation *ANIMS[] = {&face, &fisherman, &runner, &sisyphus, &balloon,
                      &stargazer, &campfire, &owl, &juggler,
                      &rainbow, &fire, &rain, &heart, &snake, &smiley};

constexpr int SCENE_COUNT = 9;

}  // namespace

Animation **animationList(int &count) {
    count = sizeof(ANIMS) / sizeof(ANIMS[0]);
    return ANIMS;
}

Animation **sceneAnimationList(int &count) {
    count = SCENE_COUNT;
    return ANIMS;
}
