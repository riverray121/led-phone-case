#include "ble_service.h"

#include <NimBLEDevice.h>

BleState bleState;

namespace {

const char *SVC_UUID = "7A0B0001-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_ANIM_LIST = "7A0B0002-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_ANIM_SELECT = "7A0B0003-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_BRIGHTNESS = "7A0B0004-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_DISPLAY_INFO = "7A0B0005-63B1-4A6F-8D3A-6E1C2A5B9D01";
const char *CHR_SPEED = "7A0B0006-63B1-4A6F-8D3A-6E1C2A5B9D01";

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

// Forwards a single-byte write into a BleState pending field.
class ByteWriteCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit ByteWriteCallbacks(volatile int *target) : target_(target) {}

private:
    void onWrite(NimBLECharacteristic *chr) override {
        std::string v = chr->getValue();
        if (!v.empty()) *target_ = (uint8_t)v[0];
    }

    volatile int *target_;
};

ServerCallbacks serverCallbacks;
AnimSelectCallbacks animSelectCallbacks;
ByteWriteCallbacks brightnessCallbacks(&bleState.pendingBrightness);
ByteWriteCallbacks speedCallbacks(&bleState.pendingSpeed);

// Read/write characteristic holding one byte.
NimBLECharacteristic *createByteChr(NimBLEService *svc, const char *uuid,
                                    uint8_t initial,
                                    NimBLECharacteristicCallbacks *callbacks) {
    NimBLECharacteristic *chr =
        svc->createCharacteristic(uuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    chr->setValue(&initial, 1);
    chr->setCallbacks(callbacks);
    return chr;
}

}  // namespace

void bleBegin(const char *animNamesCsv, int animCount, uint8_t initialAnim,
              uint8_t initialBrightness, uint8_t initialSpeed,
              const uint8_t displayInfo[5]) {
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

    createByteChr(svc, CHR_BRIGHTNESS, initialBrightness, &brightnessCallbacks);
    createByteChr(svc, CHR_SPEED, initialSpeed, &speedCallbacks);

    NimBLECharacteristic *di =
        svc->createCharacteristic(CHR_DISPLAY_INFO, NIMBLE_PROPERTY::READ);
    di->setValue(displayInfo, 5);

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
