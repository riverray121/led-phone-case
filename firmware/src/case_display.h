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
    // BLE DisplayInfo payload: type (1=TFT, 2=matrix), width, height, bits/px
    void info(uint8_t out[4]) const;

private:
    GFXcanvas16 canvas_{WIDTH, HEIGHT};
    uint8_t brightness_;
};
