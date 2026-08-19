// Wiring test for Case A: 1.44" ST7735 TFT (M029) on ESP32-C3 SuperMini.
// Exercises every wire: SCK/MOSI/CS/DC via draws, RES via init, BL via PWM.
// Pin map matches DESIGN.md.

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

constexpr int PIN_SCK = 4;
constexpr int PIN_MOSI = 6;
constexpr int PIN_CS = 7;
constexpr int PIN_DC = 5;
constexpr int PIN_RST = 10;
constexpr int PIN_BL = 1;

Adafruit_ST7735 tft(PIN_CS, PIN_DC, PIN_RST);

void setup() {
    Serial.begin(115200);
    delay(2000);  // give USB CDC time to enumerate before first prints
    Serial.println("wiring-test: boot");

    pinMode(PIN_BL, OUTPUT);
    analogWrite(PIN_BL, 255);

    SPI.begin(PIN_SCK, -1, PIN_MOSI, -1);
    tft.initR(INITR_144GREENTAB);  // 1.44" 128x128 variant
    Serial.println("wiring-test: display initialized");
}

void showLabel(const char *name, uint16_t fill, uint16_t textColor) {
    tft.fillScreen(fill);
    tft.setCursor(10, 56);
    tft.setTextColor(textColor);
    tft.setTextSize(2);
    tft.print(name);
    Serial.printf("wiring-test: %s\n", name);
    delay(1200);
}

void loop() {
    showLabel("RED", ST77XX_RED, ST77XX_WHITE);
    showLabel("GREEN", ST77XX_GREEN, ST77XX_BLACK);
    showLabel("BLUE", ST77XX_BLUE, ST77XX_WHITE);
    showLabel("WHITE", ST77XX_WHITE, ST77XX_BLACK);

    // Backlight fade proves BL is on GPIO1 and PWM-controllable
    tft.fillScreen(ST77XX_YELLOW);
    tft.setCursor(10, 56);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("BL FADE");
    Serial.println("wiring-test: backlight fade");
    for (int b = 255; b >= 0; b -= 5) {
        analogWrite(PIN_BL, b);
        delay(20);
    }
    for (int b = 0; b <= 255; b += 5) {
        analogWrite(PIN_BL, b);
        delay(20);
    }
}
