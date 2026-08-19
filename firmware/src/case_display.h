// Display abstraction for the case. Animations draw into an off-screen
// canvas; present() pushes the whole frame in one SPI write. Nothing outside
// this layer may touch display hardware (see DESIGN.md: Software architecture).
#pragma once

#include <Adafruit_GFX.h>

class CaseDisplay {
public:
    static constexpr int WIDTH = 128;
    static constexpr int HEIGHT = 128;

    bool begin();
    GFXcanvas16 &canvas() { return canvas_; }
    void present();
    void setBrightness(uint8_t level);
    uint8_t brightness() const { return brightness_; }

private:
    GFXcanvas16 canvas_{WIDTH, HEIGHT};
    uint8_t brightness_ = 255;
};
