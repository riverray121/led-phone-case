#include "animation_shared.h"

#include <math.h>

uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float frand(uint32_t seed) { return (hash32(seed) & 0xFFFF) / 65535.0f; }

int Frame2D::X(float a, float h) const { return (int)lroundf(ox + dx * a + ux * h); }

int Frame2D::Y(float a, float h) const { return (int)lroundf(oy + dy * a + uy * h); }

Frame2D Frame2D::upright(float x, float y) { return {x, y, 1, 0, 0, -1}; }

void Frame2D::flip() {
    dx = -dx;
    dy = -dy;
}

float EdgePath::total() { return 4 * (SL + AL); }

Frame2D EdgePath::at(float s) {
    float per = total();
    s = fmodf(s, per);
    if (s < 0) s += per;
    int q = (int)(s / (SL + AL));
    float u = s - q * (SL + AL);

    static const float sx[4] = {MARGIN + R, ANIM_W - MARGIN, ANIM_W - MARGIN - R, MARGIN};
    static const float sy[4] = {ANIM_H - MARGIN, ANIM_H - MARGIN - R, MARGIN, MARGIN + R};
    static const float sdx[4] = {1, 0, -1, 0};
    static const float sdy[4] = {0, -1, 0, 1};
    static const float cx[4] = {ANIM_W - MARGIN - R, ANIM_W - MARGIN - R, MARGIN + R,
                                MARGIN + R};
    static const float cy[4] = {ANIM_H - MARGIN - R, MARGIN + R, MARGIN + R,
                                ANIM_H - MARGIN - R};

    Frame2D f;
    if (u <= SL) {
        f.ox = sx[q] + sdx[q] * u;
        f.oy = sy[q] + sdy[q] * u;
        f.dx = sdx[q];
        f.dy = sdy[q];
    } else {
        float th = (ANIM_PI / 2) * (1 - q) - (u - SL) / R;
        f.ox = cx[q] + R * cosf(th);
        f.oy = cy[q] + R * sinf(th);
        f.dx = sinf(th);
        f.dy = -cosf(th);
    }
    f.ux = f.dy;
    f.uy = -f.dx;
    return f;
}

void drawHuman(GFXcanvas16 &c, const Frame2D &f, Pose pose, float phase, uint32_t ms,
               uint16_t col) {
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
                float ph = phase + k * ANIM_PI;
                float amp = pose == Pose::Push ? 0.55f : 0.9f;
                leg(sinf(ph) * amp,
                    fmaxf(0.f, sinf(ph + 0.8f)) * (pose == Pose::Push ? 0.7f : 1.2f));
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
                float ua = sinf(phase + ANIM_PI + k * ANIM_PI) * 0.75f;
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

void drawSeated(GFXcanvas16 &c, int x, int y, uint16_t col) {
    c.drawLine(x, y, x - 4, y - 10, col);            // torso, leaning back
    c.fillCircle(x - 4, y - 13, 3, col);             // head, tipped skyward
    c.drawLine(x, y, x + 6, y - 4, col);             // thigh raised
    c.drawLine(x + 6, y - 4, x + 8, y + 1, col);     // shin to the grass
    c.drawLine(x - 4, y - 10, x - 9, y + 1, col);    // propping arm
    c.drawLine(x - 4, y - 10, x - 6, y + 1, col);    // second arm
}

void drawNightSky(GFXcanvas16 &c, uint32_t ms, int skyBottomY) {
    int maxY = skyBottomY - 4;
    if (maxY < 8) maxY = 8;
    for (int i = 0; i < 26; i++) {
        int sx = (int)(frand(i * 13 + 3) * 124) + 2;
        int sy = (int)(frand(i * 13 + 4) * (maxY - 2)) + 2;
        float tw = sinf(ms * 0.0025f + i * 2.1f);
        uint16_t col = tw > 0.4f ? COL_WHITE : (tw > -0.4f ? COL_SKY_STAR : COL_DIMGRAY);
        c.drawPixel(sx, sy, col);
        if (tw > 0.8f) {
            c.drawPixel(sx - 1, sy, COL_SKY_STAR);
            c.drawPixel(sx + 1, sy, COL_SKY_STAR);
        }
    }
    c.fillCircle(106, 18, 9, COL_MOON);
    c.fillCircle(110, 15, 8, COL_BLACK);
}
