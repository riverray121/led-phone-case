#ifdef CASE_DISPLAY_MATRIX

#include "case_display.h"

#include <FastLED.h>

// Case B driver: 8x8 WS2812 matrix (D038) behind a 74HCT04 level shifter.
// Animations render into the shared 128x128 canvas; present() averages each
// 16x16 block down to one LED, so animation code never knows the resolution.

namespace {
constexpr int PIN_DATA = 3;  // through 330R into the 74HCT04
constexpr int GRID = 8;
constexpr int BLOCK = CaseDisplay::WIDTH / GRID;  // 16
// Panel wiring assumptions; flip after a hardware test if the image is
// mirrored or woven. SERPENTINE: odd rows run right-to-left.
constexpr bool SERPENTINE = false;

CRGB leds[GRID * GRID];

int ledIndex(int gx, int gy) {
    if (SERPENTINE && (gy & 1)) return gy * GRID + (GRID - 1 - gx);
    return gy * GRID + gx;
}
}  // namespace

bool CaseDisplay::begin() {
    brightness_ = 30;
    FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, GRID * GRID);
    // the phone supplies ~900 mA total; never let the panel take more than 600
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 600);
    FastLED.setBrightness(brightness_);
    FastLED.clear(true);
    return true;
}

void CaseDisplay::present() {
    const uint16_t *buf = canvas_.getBuffer();
    for (int gy = 0; gy < GRID; gy++) {
        for (int gx = 0; gx < GRID; gx++) {
            uint32_t r = 0, g = 0, b = 0;
            const uint16_t *row = buf + gy * BLOCK * WIDTH + gx * BLOCK;
            for (int y = 0; y < BLOCK; y++) {
                for (int x = 0; x < BLOCK; x++) {
                    uint16_t c = row[y * WIDTH + x];
                    r += (c >> 11) << 3;
                    g += ((c >> 5) & 0x3F) << 2;
                    b += (c & 0x1F) << 3;
                }
            }
            constexpr int N = BLOCK * BLOCK;  // 256 pixels per LED
            leds[ledIndex(gx, gy)] = CRGB(r / N, g / N, b / N);
        }
    }
    FastLED.show();
}

void CaseDisplay::setBrightness(uint8_t level) {
    brightness_ = level;
    // the power cap in begin() still bounds actual current draw
    FastLED.setBrightness(level);
}

void CaseDisplay::info(uint8_t out[4]) const {
    out[0] = 2;
    out[1] = GRID;
    out[2] = GRID;
    out[3] = 24;
}

#endif  // CASE_DISPLAY_MATRIX
