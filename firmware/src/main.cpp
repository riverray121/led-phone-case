// LED phone case firmware v0, Case A (TFT). Runs a selectable animation on
// the display; selection and brightness are controlled over BLE.

#include <Arduino.h>

#include "animations.h"
#include "ble_service.h"
#include "case_display.h"

namespace {

constexpr uint32_t FRAME_MS = 33;  // ~30 fps

CaseDisplay display;
Animation **anims;
int animCount = 0;
int currentAnim = 0;
uint32_t animStart = 0;
String namesCsv;

}  // namespace

void setup() {
    Serial.begin(115200);
    display.begin();

    anims = animationList(animCount);
    for (int i = 0; i < animCount; i++) {
        if (i) namesCsv += ',';
        namesCsv += anims[i]->name();
    }

    bleBegin(namesCsv.c_str(), animCount, currentAnim, display.brightness());
    animStart = millis();
    Serial.printf("firmware v0: %d animations: %s\n", animCount, namesCsv.c_str());
}

void loop() {
    uint32_t frameBegin = millis();

    if (bleState.pendingAnim >= 0) {
        currentAnim = bleState.pendingAnim;
        bleState.pendingAnim = -1;
        animStart = frameBegin;
        bleNotifyAnim(currentAnim);
        Serial.printf("anim -> %s\n", anims[currentAnim]->name());
    }
    if (bleState.pendingBrightness >= 0) {
        display.setBrightness(bleState.pendingBrightness);
        bleState.pendingBrightness = -1;
    }

    GFXcanvas16 &c = display.canvas();
    c.fillScreen(0x0000);
    anims[currentAnim]->frame(c, frameBegin - animStart);
    display.present();

    uint32_t spent = millis() - frameBegin;
    if (spent < FRAME_MS) delay(FRAME_MS - spent);
}
