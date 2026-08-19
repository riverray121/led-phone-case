#include "animations.h"

#include <math.h>

namespace {

constexpr int W = 128;
constexpr int H = 128;
constexpr float PI_F = 3.14159265f;

constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t GRAY = 0x8410;
constexpr uint16_t DIMGRAY = 0x39E7;
constexpr uint16_t RED = 0xF800;
constexpr uint16_t YELLOW = 0xFFE0;
constexpr uint16_t MOON = 0xFFF2;
constexpr uint16_t WATER = 0x0119;
constexpr uint16_t WAVE = 0x1C9F;
constexpr uint16_t WOOD = 0xA285;
constexpr uint16_t HILL = 0x01E2;
constexpr uint16_t SKY_STAR = 0x7BEF;

uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float frand(uint32_t seed) { return (hash32(seed) & 0xFFFF) / 65535.0f; }

// -------------------------------------------------------------- local frame
// Figures are drawn in a local frame: a = forward along travel, h = up away
// from the ground, so the same body code works on any edge or slope.

struct Frame2D {
    float ox, oy, dx, dy, ux, uy;

    int X(float a, float h) const { return (int)lroundf(ox + dx * a + ux * h); }
    int Y(float a, float h) const { return (int)lroundf(oy + dy * a + uy * h); }

    static Frame2D upright(float x, float y) {
        return {x, y, 1, 0, 0, -1};
    }

    void flip() { dx = -dx; dy = -dy; }
};

// ------------------------------------------------------- edge path (rounded)
// Perimeter path hugging the screen edge, with quarter-circle corners so a
// walker's orientation turns smoothly instead of snapping 90 degrees.

struct EdgePath {
    static constexpr float MARGIN = 2;
    static constexpr float R = 16;
    static constexpr float SL = W - 2 * MARGIN - 2 * R;  // straight side length
    static constexpr float AL = R * PI_F / 2;            // corner arc length

    static float total() { return 4 * (SL + AL); }

    static Frame2D at(float s) {
        float per = total();
        s = fmodf(s, per);
        if (s < 0) s += per;
        int q = (int)(s / (SL + AL));
        float u = s - q * (SL + AL);

        static const float sx[4] = {MARGIN + R, W - MARGIN, W - MARGIN - R, MARGIN};
        static const float sy[4] = {H - MARGIN, H - MARGIN - R, MARGIN, MARGIN + R};
        static const float sdx[4] = {1, 0, -1, 0};
        static const float sdy[4] = {0, -1, 0, 1};
        static const float cx[4] = {W - MARGIN - R, W - MARGIN - R, MARGIN + R, MARGIN + R};
        static const float cy[4] = {H - MARGIN - R, MARGIN + R, MARGIN + R, H - MARGIN - R};

        Frame2D f;
        if (u <= SL) {
            f.ox = sx[q] + sdx[q] * u;
            f.oy = sy[q] + sdy[q] * u;
            f.dx = sdx[q];
            f.dy = sdy[q];
        } else {
            float th = (PI_F / 2) * (1 - q) - (u - SL) / R;
            f.ox = cx[q] + R * cosf(th);
            f.oy = cy[q] + R * sinf(th);
            f.dx = sinf(th);
            f.dy = -cosf(th);
        }
        f.ux = f.dy;
        f.uy = -f.dx;
        return f;
    }
};

// ------------------------------------------------------ articulated figure
// Two-segment limbs: thighs and shins with a knee, upper arms and forearms
// with an elbow. Roughly 23 px tall.

enum class Pose { Run, Stand, Look, ArmsUp, Jump, Slump, Push };

void drawHuman(GFXcanvas16 &c, const Frame2D &f, Pose pose, float phase,
               uint32_t ms, uint16_t col) {
    float lean = 0, shoulderH = 16, headA = 0, headH = 20;
    switch (pose) {
        case Pose::Run:    lean = 1.6f; headA = 2.0f; break;
        case Pose::ArmsUp: lean = 1.0f; headA = 1.3f; break;
        case Pose::Jump:   lean = 1.0f; headA = 1.3f; break;
        case Pose::Push:   lean = 4.5f; shoulderH = 14; headA = 7.0f; headH = 17; break;
        case Pose::Slump:  lean = 1.5f; shoulderH = 14; headA = 3.5f; headH = 15.5f; break;
        default:           break;
    }
    if (pose == Pose::Look) headA = sinf(ms * 0.004f) * 2.8f;

    // legs
    auto leg = [&](float thigh, float bend) {
        float ka = sinf(thigh) * 5.5f, kh = 9 - cosf(thigh) * 5.5f;
        float shin = thigh - bend;
        float fa = ka + sinf(shin) * 5.0f, fh = kh - cosf(shin) * 5.0f;
        if (fh < -0.5f) fh = -0.5f;
        c.drawLine(f.X(0, 9), f.Y(0, 9), f.X(ka, kh), f.Y(ka, kh), col);
        c.drawLine(f.X(ka, kh), f.Y(ka, kh), f.X(fa, fh), f.Y(fa, fh), col);
    };
    switch (pose) {
        case Pose::Run:
        case Pose::ArmsUp:
        case Pose::Push:
            for (int k = 0; k < 2; k++) {
                float ph = phase + k * PI_F;
                float amp = pose == Pose::Push ? 0.55f : 0.9f;
                leg(sinf(ph) * amp, fmaxf(0.f, sinf(ph + 0.8f)) * (pose == Pose::Push ? 0.7f : 1.2f));
            }
            break;
        case Pose::Jump:
            leg(1.15f, 2.1f);
            leg(0.9f, 1.9f);
            break;
        default:  // Stand, Look, Slump
            leg(0.18f, 0.15f);
            leg(-0.18f, 0.05f);
            break;
    }

    // torso and head
    c.drawLine(f.X(0, 9), f.Y(0, 9), f.X(lean, shoulderH), f.Y(lean, shoulderH), col);
    c.fillCircle(f.X(headA, headH), f.Y(headA, headH), 3, col);

    // arms
    auto arm = [&](float upper, float fore) {
        float ea = lean + sinf(upper) * 4.5f, eh = shoulderH - cosf(upper) * 4.5f;
        float ha = ea + sinf(fore) * 4.0f, hh = eh - cosf(fore) * 4.0f;
        c.drawLine(f.X(lean, shoulderH), f.Y(lean, shoulderH), f.X(ea, eh), f.Y(ea, eh), col);
        c.drawLine(f.X(ea, eh), f.Y(ea, eh), f.X(ha, hh), f.Y(ha, hh), col);
    };
    switch (pose) {
        case Pose::Run:
            for (int k = 0; k < 2; k++) {
                float ua = sinf(phase + PI_F + k * PI_F) * 0.75f;
                arm(ua, ua + 1.2f);
            }
            break;
        case Pose::ArmsUp:
        case Pose::Jump:
            for (int k = 0; k < 2; k++) {
                float ua = 2.35f + k * 0.3f + sinf(ms * 0.01f + k * 2) * 0.15f;
                arm(ua, ua + 0.35f);
            }
            break;
        case Pose::Push:
            arm(1.35f, 1.05f);
            arm(1.75f, 1.45f);
            break;
        case Pose::Slump:
            arm(0.25f, 0.15f);
            arm(-0.15f, -0.05f);
            break;
        default:  // Stand, Look
            arm(0.15f, 0.1f);
            arm(-0.15f, -0.1f);
            break;
    }
}

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
                c.fillRect(ex - 20, 48, 41, 4, WHITE);
            } else {
                c.fillCircle(ex, 50, 20, WHITE);
                c.fillCircle(ex + (int)(px * 9), 50 + (int)(py * 8), 8, BLACK);
            }
        }

        if (seg % 6 == 4) {
            c.fillCircle(64, 100, 9, WHITE);
            c.fillCircle(64, 100, 5, BLACK);
        } else {
            for (int x = -22; x <= 22; x += 2) {
                int y = 96 + (int)(10 - (x * x) / 48.0f);
                c.fillRect(64 + x, y, 3, 3, WHITE);
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
        c.fillCircle(100, 22, 10, MOON);
        c.fillCircle(105, 19, 9, BLACK);
        for (int i = 0; i < 9; i++) {
            int sx = (int)(frand(i * 7 + 1) * 120) + 4;
            int sy = (int)(frand(i * 7 + 2) * 60) + 4;
            uint16_t col = (ms / 400 + i) % 4 ? SKY_STAR : WHITE;
            c.drawPixel(sx, sy, col);
        }

        c.fillRect(0, 86, W, H - 86, WATER);
        for (int row = 0; row < 4; row++) {
            int baseY = 90 + row * 9;
            for (int x = 0; x < W; x += 3) {
                float ph = ms * 0.002f + x * 0.12f + row * 1.7f;
                c.drawPixel(x, baseY + (int)(sinf(ph) * 2), WAVE);
            }
        }

        int xb = -60 + (int)((ms / 55) % 250);
        int bob = (int)(sinf(ms * 0.003f) * 1.5f);
        int yb = 84 + bob;

        c.fillTriangle(xb, yb, xb + 8, yb + 6, xb + 44, yb, WOOD);
        c.fillTriangle(xb + 8, yb + 6, xb + 36, yb + 6, xb + 44, yb, WOOD);

        int xf = xb + 36;
        int fy = yb;
        c.drawLine(xf - 2, fy, xf, fy - 7, WHITE);
        c.drawLine(xf + 2, fy, xf, fy - 7, WHITE);
        c.drawLine(xf, fy - 7, xf - 1, fy - 16, WHITE);
        c.fillCircle(xf - 1, fy - 19, 2, WHITE);
        c.fillTriangle(xf - 6, fy - 21, xf + 4, fy - 21, xf - 1, fy - 25, YELLOW);

        float stroke = sinf(ms * 0.004f) * 0.5f + 0.25f;
        int hx = xf - 3, hy = fy - 13;
        int px2 = hx - (int)(sinf(stroke) * 26);
        int py2 = hy + (int)(cosf(stroke) * 26);
        c.drawLine(hx, hy - 3, px2, py2, WOOD);
        c.drawLine(xf + 1, fy - 12, hx, hy - 3, WHITE);
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
            drawHuman(c, f, Pose::Run, dist * 0.55f, ms, WHITE);
        } else {
            drawHuman(c, f, Pose::Look, 0, ms, WHITE);
            if ((lookMs / 400) % 2) {
                c.setTextColor(YELLOW);
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
        c.fillCircle(bx, by, (int)BR, GRAY);
        for (int k = 0; k < 3; k++) {
            float a = rot + k * 2.094f;
            c.drawLine(bx, by, bx + (int)(cosf(a) * (BR - 3)),
                       by + (int)(sinf(a) * (BR - 3)), DIMGRAY);
        }

        Frame2D ff = EdgePath::at(base + uf - GAP);
        switch (state) {
            case PUSHING:
                drawHuman(c, ff, Pose::Push, (base + uf) * 0.5f, ms, WHITE);
                break;
            case WATCHING:
                drawHuman(c, ff, Pose::Slump, 0, ms, WHITE);
                break;
            case RETURNING:
                ff.flip();  // he walks facing the way he trudges
                drawHuman(c, ff, Pose::Run, (base + uf) * 0.4f, ms, WHITE);
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

        c.fillCircle(bx, by, 9, RED);
        c.fillTriangle(bx - 3, by + 9, bx + 3, by + 9, bx, by + 12, RED);
        for (int i = 0; i < 16; i++) {
            c.drawPixel(bx + (int)(sinf(ms * 0.004f + i * 0.6f) * 2),
                        by + 12 + i, WHITE);
        }

        c.drawLine(0, 105, W, 105, DIMGRAY);

        // the kid sprints after it and leaps mid-screen, never catching it
        float feetY = 104;
        Pose pose = Pose::ArmsUp;
        if (bx > 55 && bx < 90) {
            feetY -= sinf((bx - 55) * PI_F / 35.0f) * 13;
            pose = Pose::Jump;
        }
        Frame2D f = Frame2D::upright(bx - 30, feetY);
        drawHuman(c, f, pose, ms * 0.014f, ms, WHITE);
    }
};

// ---------------------------------------------------------------- Stargazer

class StargazerAnim : public Animation {
public:
    const char *name() const override { return "Stargazer"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        for (int i = 0; i < 26; i++) {
            int sx = (int)(frand(i * 13 + 3) * 124) + 2;
            int sy = (int)(frand(i * 13 + 4) * 92) + 2;
            float tw = sinf(ms * 0.0025f + i * 2.1f);
            uint16_t col = tw > 0.4f ? WHITE : (tw > -0.4f ? SKY_STAR : DIMGRAY);
            c.drawPixel(sx, sy, col);
            if (tw > 0.8f) {
                c.drawPixel(sx - 1, sy, SKY_STAR);
                c.drawPixel(sx + 1, sy, SKY_STAR);
            }
        }
        c.fillCircle(106, 18, 9, MOON);
        c.fillCircle(110, 15, 8, BLACK);

        uint32_t sp = ms % 7000;
        if (sp < 700) {
            uint32_t ep = ms / 7000;
            int ox = (int)(frand(ep + 21) * 70) + 10;
            int oy = (int)(frand(ep + 22) * 30) + 8;
            int d = sp / 14;
            c.drawLine(ox + d - 8, oy + d / 2 - 4, ox + d, oy + d / 2, WHITE);
        }

        c.fillCircle(64, 176, 74, HILL);
        seated(c, 46, 104);
        seated(c, 74, 104);
    }

private:
    // seated figure leaning back on their arms, face tilted up at the sky
    static void seated(GFXcanvas16 &c, int x, int y) {
        c.drawLine(x, y, x - 4, y - 10, WHITE);        // torso, leaning back
        c.fillCircle(x - 4, y - 13, 3, WHITE);         // head, tipped skyward
        c.drawLine(x, y, x + 6, y - 4, WHITE);         // thigh raised
        c.drawLine(x + 6, y - 4, x + 8, y + 1, WHITE); // shin to the grass
        c.drawLine(x - 4, y - 10, x - 9, y + 1, WHITE);  // propping arm
        c.drawLine(x - 4, y - 10, x - 6, y + 1, WHITE);  // second arm
    }
};

FaceAnim face;
FishermanAnim fisherman;
RunnerAnim runner;
SisyphusAnim sisyphus;
BalloonAnim balloon;
StargazerAnim stargazer;

Animation *ANIMS[] = {&face, &fisherman, &runner, &sisyphus, &balloon, &stargazer};

}  // namespace

Animation **animationList(int &count) {
    count = sizeof(ANIMS) / sizeof(ANIMS[0]);
    return ANIMS;
}
