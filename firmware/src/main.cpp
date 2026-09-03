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
uint8_t speed = SPEED_ONE;
uint32_t animMs = 0;  // animation clock, advanced by real time scaled by speed
uint32_t lastLoop = 0;
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

    int sceneCount = 0;
    sceneAnimationList(sceneCount);

    uint8_t displayInfo[5];
    display.info(displayInfo);
    displayInfo[4] = (uint8_t)sceneCount;
    bleBegin(namesCsv.c_str(), animCount, currentAnim, display.brightness(), speed,
             displayInfo);
    lastLoop = millis();
    Serial.printf("firmware v0: %d animations: %s\n", animCount, namesCsv.c_str());
}

void loop() {
    uint32_t frameBegin = millis();
    animMs += (frameBegin - lastLoop) * speed / SPEED_ONE;
    lastLoop = frameBegin;

    if (bleState.pendingAnim >= 0) {
        currentAnim = bleState.pendingAnim;
        bleState.pendingAnim = -1;
        animMs = 0;
        bleNotifyAnim(currentAnim);
        Serial.printf("anim -> %s\n", anims[currentAnim]->name());
    }
    if (bleState.pendingBrightness >= 0) {
        display.setBrightness(bleState.pendingBrightness);
        bleState.pendingBrightness = -1;
    }
    if (bleState.pendingSpeed >= 0) {
        speed = bleState.pendingSpeed;
        bleState.pendingSpeed = -1;
    }

    GFXcanvas16 &c = display.canvas();
    c.fillScreen(0x0000);
    anims[currentAnim]->frame(c, animMs);
    display.present();

    uint32_t spent = millis() - frameBegin;
    if (spent < FRAME_MS) delay(FRAME_MS - spent);
}
