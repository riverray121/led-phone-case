#include "case_display.h"

#include <Adafruit_ST7735.h>
#include <SPI.h>

namespace {
constexpr int PIN_SCK = 4;
constexpr int PIN_MOSI = 6;
constexpr int PIN_CS = 7;
constexpr int PIN_DC = 5;
constexpr int PIN_RST = 10;
constexpr int PIN_BL = 1;
// miso is unused (display is write-only); parked on free GPIO0 so the
// ESP32 core doesn't log an error about a missing default pin
constexpr int PIN_MISO_PARK = 0;
// 1 = 90 degrees clockwise so the panel reads upright on the phone back
constexpr uint8_t ROTATION = 1;

Adafruit_ST7735 tft(PIN_CS, PIN_DC, PIN_RST);
}  // namespace

bool CaseDisplay::begin() {
    pinMode(PIN_BL, OUTPUT);
    analogWrite(PIN_BL, brightness_);

    SPI.begin(PIN_SCK, PIN_MISO_PARK, PIN_MOSI, -1);
    tft.initR(INITR_144GREENTAB);
    tft.setSPISpeed(40000000);
    tft.setRotation(ROTATION);
    tft.fillScreen(0x0000);
    return true;
}

void CaseDisplay::present() {
    tft.drawRGBBitmap(0, 0, canvas_.getBuffer(), WIDTH, HEIGHT);
}

void CaseDisplay::setBrightness(uint8_t level) {
    brightness_ = level;
    analogWrite(PIN_BL, level);
}
