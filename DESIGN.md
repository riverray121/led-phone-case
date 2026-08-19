# LED Phone Case

Clear iPhone 16 Pro case with a low-resolution display on the back, driven by an ESP32 and controlled from a companion iOS app over BLE.

Two hardware variants are built in parallel:

- **Case A**: 1.44" TFT LCD (full color, 128×128)
- **Case B**: 8×8 WS2812 LED matrix

## Parts

### Case A

| Part | Notes |
|---|---|
| ESP32-C3 SuperMini | Power in, BLE, drives display |
| 1.44" TFT LCD 128×128 SPI (M029) | Full-color display |
| Clear TPU case | iPhone 16 Pro |

### Case B

| Part | Notes |
|---|---|
| ESP32-C3 SuperMini | Power in, BLE, drives matrix |
| 8×8 WS2812 matrix (D038) | 64 addressable RGB LEDs |
| 74HCT04 | Level shifter for LED data |
| 1000 µF / 10 V electrolytic cap | Across matrix 5V/GND |
| 330 Ω resistor | Series on data line |
| 1N4148 diode | Fallback if the 74HCT04 fails |
| Clear TPU case | iPhone 16 Pro |

### Shared

- USB-C to USB-C cable, 20 cm
- Jumper wires
- Headers

## What each part does

- **SuperMini**: power in, BLE, drives the display. Provides a regulated 3V3 rail and a raw-USB 5V rail.
- **TFT**: SPI, runs off 3V3, draws ~40 mA.
- **Matrix**: runs off 5V, one data wire.
- **74HCT04**: shifts the 3.3 V data signal up to 5 V so the LEDs read it reliably. Two inverters in series.
- **330 Ω**: series resistor on the data line.
- **1000 µF**: across matrix 5V/GND, absorbs the switch-on current spike. Striped leg to GND.
- **1N4148**: fallback if the 74HCT04 fails. Goes in series on the matrix 5 V supply instead, dropping the LED supply so 3.3 V data clears the input threshold.

## Wiring

### Case A

| TFT | ESP32-C3 |
|---|---|
| GND | G |
| VCC | 3V3 |
| SCL (clock) | GPIO4 |
| SDA (MOSI) | GPIO6 |
| RES | GPIO10 |
| DC | GPIO5 |
| CS | GPIO7 |
| BL | GPIO1 (PWM brightness control; if BL drives the backlight LEDs directly at >20 mA, buffer with a transistor) |

Avoid GPIO2, 8, 9 (strapping pins).

### Case B

Matrix:

- 5V → 5V
- GND → G
- DIN → 74HCT04 pin 4

74HCT04:

- Pin 14 → 5V
- Pin 7 → GND
- Pin 1 → GPIO3 via 330 Ω
- Pin 2 → pin 3 (jumper)
- Pin 4 → matrix DIN
- Pins 5, 9, 11, 13 → GND (unused inputs tied low)

## Power

iPhone USB-C → cable → SuperMini. No battery. About 900 mA is available from the phone; the matrix at full white wants 3.8 A, so cap it in firmware:

```cpp
FastLED.setMaxPowerInVoltsAndMilliamps(5, 600);
FastLED.setBrightness(30);
```

Bench-test on a power bank first.

## App

BLE, not the cable: iOS blocks USB accessory data without MFi. Swift + CoreBluetooth on the phone, GATT server on the ESP32. Two modes:

- Stream frames live from the app
- Upload animations to ESP32 flash and play them standalone

## Display paths long-term

1. **Backlit TFT LCD** (Case A): full color, cheap, works today. 26 mm square. Needs a tinted shell to hide it when off.
2. **Transparent OLED**: Waveshare 1.51", 128×64, ~US$25. See-through when off, but monochrome blue and only 34 × 17 mm active area.
3. **Custom LED flex** (end goal): 0402 RGB LEDs on 0.1 mm polyimide, IS31FL3741 driver, under 0.6 mm thick, full-back coverage. Needs PCB design and a PCBA run.
