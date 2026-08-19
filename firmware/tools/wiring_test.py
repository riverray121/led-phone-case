# MicroPython wiring test for Case A: ST7735 1.44" TFT on ESP32-C3 SuperMini.
# Run with: mpremote run wiring_test.py
# Exercises every wire: SCK/MOSI/CS/DC via draws, RES via reset pulse, BL via PWM.
# Pin map matches DESIGN.md.

from machine import Pin, SPI, PWM
import time

bl = PWM(Pin(1), freq=5000)
bl.duty_u16(65535)

# miso is unused (display is write-only); parked on free GPIO0
spi = SPI(1, baudrate=20_000_000, sck=Pin(4), mosi=Pin(6), miso=Pin(0))
cs = Pin(7, Pin.OUT, value=1)
dc = Pin(5, Pin.OUT, value=0)
rst = Pin(10, Pin.OUT, value=1)


def cmd(c, data=b""):
    cs(0)
    dc(0)
    spi.write(bytes([c]))
    if data:
        dc(1)
        spi.write(data)
    cs(1)


rst(0)
time.sleep_ms(50)
rst(1)
time.sleep_ms(150)

cmd(0x01)  # SWRESET
time.sleep_ms(150)
cmd(0x11)  # SLPOUT
time.sleep_ms(255)
cmd(0x3A, b"\x05")  # COLMOD: 16-bit color
cmd(0x36, b"\xc8")  # MADCTL: row/col order + BGR
cmd(0x29)  # DISPON
time.sleep_ms(100)
print("display initialized")


def fill(color565):
    # 0..131 covers the panel regardless of the module's col/row offset
    cmd(0x2A, b"\x00\x00\x00\x83")  # CASET
    cmd(0x2B, b"\x00\x00\x00\x83")  # RASET
    cmd(0x2C)  # RAMWR
    cs(0)
    dc(1)
    row = bytes([color565 >> 8, color565 & 0xFF]) * 132
    for _ in range(132):
        spi.write(row)
    cs(1)


for name, c in (("RED", 0xF800), ("GREEN", 0x07E0), ("BLUE", 0x001F), ("WHITE", 0xFFFF)):
    print(name)
    fill(c)
    time.sleep(1.5)

print("backlight fade")
for d in range(65535, -1, -4096):
    bl.duty_u16(max(d, 0))
    time.sleep_ms(40)
for d in range(0, 65536, 4096):
    bl.duty_u16(min(d, 65535))
    time.sleep_ms(40)
bl.duty_u16(65535)
print("DONE - all wires exercised")
