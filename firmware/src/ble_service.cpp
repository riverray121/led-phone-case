#include "ble_service.h"

#include <NimBLEDevice.h>

BleState bleState;

namespace {

const char *SVC_UUID = "7A0B0001-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_ANIM_LIST = "7A0B0002-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_ANIM_SELECT = "7A0B0003-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_BRIGHTNESS = "7A0B0004-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_DISPLAY_INFO = "7A0B0005-63B1-4A6F-8D3A-6E1C2A5B9D01";

NimBLECharacteristic *animSelectChr = nullptr;
int gAnimCount = 0;

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *) override { Serial.println("ble: connected"); }
    void onDisconnect(NimBLEServer *) override {
        Serial.println("ble: disconnected, advertising again");
        NimBLEDevice::startAdvertising();
    }
};

class AnimSelectCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *chr) override {
        std::string v = chr->getValue();
        if (!v.empty() && (uint8_t)v[0] < gAnimCount) {
            bleState.pendingAnim = (uint8_t)v[0];
        }
    }
};

class BrightnessCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *chr) override {
        std::string v = chr->getValue();
        if (!v.empty()) bleState.pendingBrightness = (uint8_t)v[0];
    }
};

ServerCallbacks serverCallbacks;
AnimSelectCallbacks animSelectCallbacks;
BrightnessCallbacks brightnessCallbacks;

}  // namespace

void bleBegin(const char *animNamesCsv, int animCount, uint8_t initialAnim,
              uint8_t initialBrightness) {
    gAnimCount = animCount;

    NimBLEDevice::init("LED Case");
    NimBLEServer *server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);

    NimBLEService *svc = server->createService(SVC_UUID);

    NimBLECharacteristic *list =
        svc->createCharacteristic(CHR_ANIM_LIST, NIMBLE_PROPERTY::READ);
    list->setValue((uint8_t *)animNamesCsv, strlen(animNamesCsv));

    animSelectChr = svc->createCharacteristic(
        CHR_ANIM_SELECT,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    animSelectChr->setValue(&initialAnim, 1);
    animSelectChr->setCallbacks(&animSelectCallbacks);

    NimBLECharacteristic *bright = svc->createCharacteristic(
        CHR_BRIGHTNESS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    bright->setValue(&initialBrightness, 1);
    bright->setCallbacks(&brightnessCallbacks);

    // type 1 = TFT, 128x128, 16 bits per pixel
    uint8_t info[4] = {1, 128, 128, 16};
    NimBLECharacteristic *di =
        svc->createCharacteristic(CHR_DISPLAY_INFO, NIMBLE_PROPERTY::READ);
    di->setValue(info, sizeof(info));

    svc->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SVC_UUID);
    adv->setScanResponse(true);
    adv->start();
    Serial.println("ble: advertising as 'LED Case'");
}

void bleNotifyAnim(uint8_t index) {
    if (!animSelectChr) return;
    animSelectChr->setValue(&index, 1);
    animSelectChr->notify();
}
