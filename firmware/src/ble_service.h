// BLE GATT server. Protocol (mirrored in DESIGN.md and the iOS app):
//   service 7A0B0001-...: LED case control
//   0002 AnimList    read          comma-separated animation names
//   0003 AnimSelect  read/write/notify  uint8 index into AnimList
//   0004 Brightness  read/write    uint8 0-255
//   0005 DisplayInfo read          uint8[4]: type, width, height, bits/px
// Writes land in pending* fields; the main loop applies them so all display
// work stays on one core context.
#pragma once

#include <stdint.h>

struct BleState {
    volatile int pendingAnim = -1;
    volatile int pendingBrightness = -1;
};

extern BleState bleState;

void bleBegin(const char *animNamesCsv, int animCount, uint8_t initialAnim,
              uint8_t initialBrightness);
void bleNotifyAnim(uint8_t index);
