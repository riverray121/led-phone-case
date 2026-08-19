#include "animations.h"

#include <math.h>

namespace {

constexpr int W = 128;
constexpr int H = 128;

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

// ---------------------------------------------------------------- Face

class FaceAnim : public Animation {
public:
    const char *name() const override { return "Face"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        // where the pupils point changes every 2s, easing to the new spot
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

        // mouth: mostly a smile, sometimes surprised
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
        // night sky
        c.fillCircle(100, 22, 10, MOON);
        c.fillCircle(105, 19, 9, BLACK);
        for (int i = 0; i < 9; i++) {
            int sx = (int)(frand(i * 7 + 1) * 120) + 4;
            int sy = (int)(frand(i * 7 + 2) * 60) + 4;
            uint16_t col = (ms / 400 + i) % 4 ? SKY_STAR : WHITE;
            c.drawPixel(sx, sy, col);
        }

        // water with slowly scrolling waves
        c.fillRect(0, 86, W, H - 86, WATER);
        for (int row = 0; row < 4; row++) {
            int baseY = 90 + row * 9;
            for (int x = 0; x < W; x += 3) {
                float ph = ms * 0.002f + x * 0.12f + row * 1.7f;
                c.drawPixel(x, baseY + (int)(sinf(ph) * 2), WAVE);
            }
        }

        // boat drifts across and wraps; everything bobs together
        int xb = -60 + (int)((ms / 55) % 250);
        int bob = (int)(sinf(ms * 0.003f) * 1.5f);
        int yb = 84 + bob;

        c.fillTriangle(xb, yb, xb + 8, yb + 6, xb + 44, yb, WOOD);
        c.fillTriangle(xb + 8, yb + 6, xb + 36, yb + 6, xb + 44, yb, WOOD);

        // the rower stands at the stern with a long pole
        int xf = xb + 36;
        int fy = yb;
        c.drawLine(xf - 2, fy, xf, fy - 7, WHITE);   // legs
        c.drawLine(xf + 2, fy, xf, fy - 7, WHITE);
        c.drawLine(xf, fy - 7, xf - 1, fy - 16, WHITE);  // body, leaning
        c.fillCircle(xf - 1, fy - 19, 2, WHITE);         // head
        c.fillTriangle(xf - 6, fy - 21, xf + 4, fy - 21, xf - 1, fy - 25,
                       YELLOW);  // conical hat

        // rowing stroke: pole pivots around the hands
        float stroke = sinf(ms * 0.004f) * 0.5f + 0.25f;
        int hx = xf - 3, hy = fy - 13;
        int px2 = hx - (int)(sinf(stroke) * 26);
        int py2 = hy + (int)(cosf(stroke) * 26);
        c.drawLine(hx, hy - 3, px2, py2, WOOD);
        c.drawLine(xf + 1, fy - 12, hx, hy - 3, WHITE);  // arms to pole
    }
};

// ------------------------------------------------- perimeter walk helper

// Maps a distance along the inside of the screen border to a position plus
// direction/inward-normal vectors, so figures can walk on all four edges.
struct EdgeWalker {
    static constexpr int M = 14;
    static constexpr int L = W - 2 * M;  // one side, 100 px

    float posX, posY, dirX, dirY, upX, upY;

    void at(float s) {
        while (s < 0) s += 4 * L;
        int side = (int)(s / L) % 4;
        float u = fmodf(s, (float)L);
        switch (side) {
            case 0: posX = M + u;       posY = H - M;    dirX = 1;  dirY = 0;  break;
            case 1: posX = W - M;       posY = H - M - u; dirX = 0; dirY = -1; break;
            case 2: posX = W - M - u;   posY = M;        dirX = -1; dirY = 0;  break;
            default: posX = M;          posY = M + u;    dirX = 0;  dirY = 1;  break;
        }
        upX = -dirY;  // inward normal: rotate direction toward screen center
        upY = dirX;
        if (side == 1) { upX = -1; upY = 0; }
        if (side == 3) { upX = 1; upY = 0; }
        if (side == 0) { upX = 0; upY = -1; }
        if (side == 2) { upX = 0; upY = 1; }
    }

    int x(float a, float h) const { return (int)(posX + dirX * a + upX * h); }
    int y(float a, float h) const { return (int)(posY + dirY * a + upY * h); }
};

void drawStickRunner(GFXcanvas16 &c, const EdgeWalker &w, int gait,
                     bool armsUp, uint16_t col) {
    // gait 0/1 alternates legs and arms; gait -1 stands
    float l1 = gait < 0 ? -2 : (gait ? 4 : -4);
    float l2 = gait < 0 ? 2 : (gait ? -4 : 4);
    c.drawLine(w.x(0, 6), w.y(0, 6), w.x(l1, 0), w.y(l1, 0), col);
    c.drawLine(w.x(0, 6), w.y(0, 6), w.x(l2, 0), w.y(l2, 0), col);
    c.drawLine(w.x(0, 6), w.y(0, 6), w.x(0, 11), w.y(0, 11), col);
    if (armsUp) {
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(3, 14), w.y(3, 14), col);
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(-3, 14), w.y(-3, 14), col);
    } else if (gait < 0) {
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(1, 5), w.y(1, 5), col);
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(-1, 5), w.y(-1, 5), col);
    } else {
        float a1 = gait ? 4 : -4;
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(a1, 7), w.y(a1, 7), col);
        c.drawLine(w.x(0, 10), w.y(0, 10), w.x(-a1, 7), w.y(-a1, 7), col);
    }
    c.fillCircle(w.x(0, 13), w.y(0, 13), 2, col);
}

// ---------------------------------------------------------------- Runner

class RunnerAnim : public Animation {
public:
    const char *name() const override { return "Runner"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.drawRect(EdgeWalker::M - 4, EdgeWalker::M - 4,
                   W - 2 * EdgeWalker::M + 8, H - 2 * EdgeWalker::M + 8,
                   DIMGRAY);

        // 10s lap, then a 2s pause to look around
        uint32_t period = ms % 12000;
        EdgeWalker w;
        if (period < 10000) {
            float s = period * (4.0f * EdgeWalker::L / 10000.0f);
            w.at(s);
            drawStickRunner(c, w, (int)(s / 7) % 2, false, WHITE);
        } else {
            uint32_t lap = ms / 12000;
            w.at(frand(lap + 5) * 4 * EdgeWalker::L);
            drawStickRunner(c, w, -1, false, WHITE);
            // puzzled head-scratch: a question mark hovering above
            uint32_t p = period - 10000;
            if ((p / 400) % 2) {
                c.setTextColor(YELLOW);
                c.setCursor(w.x(0, 22) - 2, w.y(0, 22) - 3);
                c.print('?');
            }
        }
    }
};

// ---------------------------------------------------------------- Sisyphus

class SisyphusAnim : public Animation {
public:
    const char *name() const override { return "Sisyphus"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        c.drawRect(EdgeWalker::M - 4, EdgeWalker::M - 4,
                   W - 2 * EdgeWalker::M + 8, H - 2 * EdgeWalker::M + 8,
                   DIMGRAY);

        // per-side timeline: push to 75%, boulder slips back, walk back, push on
        constexpr float L = EdgeWalker::L;
        uint32_t sideNo = ms / 6600;
        uint32_t p = ms % 6600;
        float base = (sideNo % 4) * L;
        float uf, ub;  // figure and boulder progress along the side
        bool slumped = false;
        if (p < 3000) {  // pushing up to the slip point
            uf = ub = 75.0f * p / 3000.0f;
        } else if (p < 3600) {  // boulder rolls back without him
            uf = 75;
            ub = 75 - 30.0f * (p - 3000) / 600.0f;
            slumped = true;
        } else if (p < 4400) {  // trudging back to the boulder
            uf = 75 - 30.0f * (p - 3600) / 800.0f;
            ub = 45;
            slumped = true;
        } else {  // pushing through to the corner
            uf = ub = 45 + 55.0f * (p - 4400) / 2200.0f;
        }

        EdgeWalker wf, wb;
        wf.at(base + uf - 8);
        wb.at(base + ub);

        // boulder with rotation spokes so the rolling reads
        int bx = wb.x(0, 9), by = wb.y(0, 9);
        c.fillCircle(bx, by, 9, GRAY);
        float rot = (base + ub) / 9.0f;
        for (int k = 0; k < 3; k++) {
            float a = rot + k * 2.094f;
            c.drawLine(bx, by, bx + (int)(cosf(a) * 7), by + (int)(sinf(a) * 7),
                       DIMGRAY);
        }

        // leaning figure, arms on the boulder (or slumped after the slip)
        uint16_t col = WHITE;
        int gait = slumped ? -1 : (int)((base + uf) / 6) % 2;
        float lean = slumped ? 0 : 3;
        c.drawLine(wf.x(0, 0), wf.y(0, 0), wf.x(lean, 6), wf.y(lean, 6), col);
        c.drawLine(wf.x(gait == 1 ? -4.f : -1.f, 0), wf.y(gait == 1 ? -4.f : -1.f, 0),
                   wf.x(lean, 6), wf.y(lean, 6), col);
        c.drawLine(wf.x(lean, 6), wf.y(lean, 6), wf.x(lean + 3, 10),
                   wf.y(lean + 3, 10), col);
        int headH = slumped ? 9 : 13;
        c.fillCircle(wf.x(lean + 4, headH), wf.y(lean + 4, headH), 2, col);
        if (!slumped) {
            c.drawLine(wf.x(lean + 3, 10), wf.y(lean + 3, 10), wf.x(8, 8),
                       wf.y(8, 8), col);
            c.drawLine(wf.x(lean + 3, 10), wf.y(lean + 3, 10), wf.x(8, 5),
                       wf.y(8, 5), col);
        }
    }
};

// ---------------------------------------------------------------- Balloon

class BalloonAnim : public Animation {
public:
    const char *name() const override { return "Balloon"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        int bx = -20 + (int)((ms / 45) % 190);
        int by = 42 + (int)(sinf(ms * 0.002f) * 9);

        c.fillCircle(bx, by, 7, RED);
        c.fillTriangle(bx - 2, by + 7, bx + 2, by + 7, bx, by + 10, RED);
        for (int i = 0; i < 12; i++) {  // wavy string
            c.drawPixel(bx + (int)(sinf(ms * 0.004f + i * 0.7f) * 2),
                        by + 10 + i, WHITE);
        }

        // the kid never quite catches it; jumps in the middle of the screen
        EdgeWalker w;
        w.posX = bx - 26;
        w.posY = 100;
        w.dirX = 1; w.dirY = 0; w.upX = 0; w.upY = -1;
        if (bx > 55 && bx < 85) {
            w.posY -= (int)(sinf((bx - 55) * 0.105f) * 10);
        }
        drawStickRunner(c, w, (int)(ms / 130) % 2, true, WHITE);
        c.drawLine(0, 101, W, 101, DIMGRAY);
    }
};

// ---------------------------------------------------------------- Stargazer

class StargazerAnim : public Animation {
public:
    const char *name() const override { return "Stargazer"; }

    void frame(GFXcanvas16 &c, uint32_t ms) override {
        // twinkling starfield
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

        // a shooting star every 7 seconds
        uint32_t sp = ms % 7000;
        if (sp < 700) {
            uint32_t ep = ms / 7000;
            int ox = (int)(frand(ep + 21) * 70) + 10;
            int oy = (int)(frand(ep + 22) * 30) + 8;
            int d = sp / 14;
            c.drawLine(ox + d - 8, oy + d / 2 - 4, ox + d, oy + d / 2, WHITE);
        }

        // hill with two small figures looking up
        c.fillCircle(64, 176, 74, HILL);
        for (int fx : {52, 72}) {
            c.fillRect(fx - 2, 108, 5, 6, WHITE);            // seated body
            c.fillCircle(fx + (fx < 64 ? 1 : -1), 105, 3, WHITE);  // head tilted up
        }
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
