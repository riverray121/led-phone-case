#include "Adafruit_GFX.h"

#include <stdlib.h>
#include <string.h>

#define swap(a, b)                                                             \
  do {                                                                         \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  } while (0)

Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h)
    : WIDTH(w), HEIGHT(h), _width(w), _height(h) {}

void Adafruit_GFX::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                             uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx = x1 - x0;
  int16_t dy = abs(y1 - y0);
  int16_t err = dx / 2;
  int16_t ystep = y0 < y1 ? 1 : -1;

  for (; x0 <= x1; x0++) {
    if (steep)
      writePixel(y0, x0, color);
    else
      writePixel(x0, y0, color);
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void Adafruit_GFX::writeFastVLine(int16_t x, int16_t y, int16_t h,
                                  uint16_t color) {
  writeLine(x, y, x, y + h - 1, color);
}

void Adafruit_GFX::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                  uint16_t color) {
  writeLine(x, y, x + w - 1, y, color);
}

void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                 uint16_t color) {
  startWrite();
  writeLine(x, y, x, y + h - 1, color);
  endWrite();
}

void Adafruit_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                 uint16_t color) {
  startWrite();
  writeLine(x, y, x + w - 1, y, color);
  endWrite();
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            uint16_t color) {
  startWrite();
  for (int16_t i = x; i < x + w; i++)
    writeFastVLine(i, y, h, color);
  endWrite();
}

void Adafruit_GFX::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}

void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            uint16_t color) {
  if (x0 == x1) {
    if (y0 > y1)
      swap(y0, y1);
    drawFastVLine(x0, y0, y1 - y0 + 1, color);
  } else if (y0 == y1) {
    if (x0 > x1)
      swap(x0, x1);
    drawFastHLine(x0, y0, x1 - x0 + 1, color);
  } else {
    startWrite();
    writeLine(x0, y0, x1, y1, color);
    endWrite();
  }
}

void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  startWrite();
  writePixel(x0, y0 + r, color);
  writePixel(x0, y0 - r, color);
  writePixel(x0 + r, y0, color);
  writePixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    writePixel(x0 + x, y0 + y, color);
    writePixel(x0 - x, y0 + y, color);
    writePixel(x0 + x, y0 - y, color);
    writePixel(x0 - x, y0 - y, color);
    writePixel(x0 + y, y0 + x, color);
    writePixel(x0 - y, y0 + x, color);
    writePixel(x0 + y, y0 - x, color);
    writePixel(x0 - y, y0 - x, color);
  }
  endWrite();
}

void Adafruit_GFX::drawCircleHelper(int16_t x0, int16_t y0, int16_t r,
                                    uint8_t cornername, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      writePixel(x0 + x, y0 + y, color);
      writePixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      writePixel(x0 + x, y0 - y, color);
      writePixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      writePixel(x0 - y, y0 + x, color);
      writePixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      writePixel(x0 - y, y0 - x, color);
      writePixel(x0 - x, y0 - y, color);
    }
  }
}

void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  startWrite();
  writeFastHLine(x, y, w, color);
  writeFastHLine(x, y + h - 1, w, color);
  writeFastVLine(x, y, h, color);
  writeFastVLine(x + w - 1, y, h, color);
  endWrite();
}

void Adafruit_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                                 uint16_t color) {
  int16_t max_radius = ((w < h) ? w : h) / 2;
  if (r > max_radius) r = max_radius;
  startWrite();
  writeFastHLine(x + r, y, w - 2 * r, color);
  writeFastHLine(x + r, y + h - 1, w - 2 * r, color);
  writeFastVLine(x, y + r, h - 2 * r, color);
  writeFastVLine(x + w - 1, y + r, h - 2 * r, color);
  drawCircleHelper(x + r, y + r, r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
  endWrite();
}

void Adafruit_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void Adafruit_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r,
                              uint16_t color) {
  startWrite();
  writeFastVLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
  endWrite();
}

void Adafruit_GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
                                    uint8_t corners, int16_t delta,
                                    uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++;
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (x < (y + 1)) {
      if (corners & 1)
        writeFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
      if (corners & 2)
        writeFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
    }
    if (y != py) {
      if (corners & 1)
        writeFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
      if (corners & 2)
        writeFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
      py = y;
    }
    px = x;
  }
}

void Adafruit_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                int16_t x2, int16_t y2, uint16_t color) {
  int16_t a, b, y, last;

  if (y0 > y1) {
    swap(y0, y1);
    swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1);
    swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1);
    swap(x0, x1);
  }

  startWrite();
  if (y0 == y2) {
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    writeFastHLine(a, y0, b - a + 1, color);
    endWrite();
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  if (y1 == y2)
    last = y1;
  else
    last = y1 - 1;

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    if (a > b)
      swap(a, b);
    writeFastHLine(a, y, b - a + 1, color);
  }

  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    if (a > b)
      swap(a, b);
    writeFastHLine(a, y, b - a + 1, color);
  }
  endWrite();
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w,
                              int16_t h, uint16_t color) {
  int16_t byteWidth = (w + 7) / 8;
  uint8_t b = 0;

  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        b <<= 1;
      else
        b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      if (b & 0x80) writePixel(x + i, y, color);
    }
  }
  endWrite();
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w,
                              int16_t h, uint16_t color, uint16_t bg) {
  int16_t byteWidth = (w + 7) / 8;
  uint8_t b = 0;

  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        b <<= 1;
      else
        b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      writePixel(x + i, y, (b & 0x80) ? color : bg);
    }
  }
  endWrite();
}

void Adafruit_GFX::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w,
                                 int16_t h) {
  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      writePixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
    }
  }
  endWrite();
}

void Adafruit_GFX::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w,
                                 int16_t h) {
  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      writePixel(x + i, y, bitmap[j * w + i]);
    }
  }
  endWrite();
}

void Adafruit_GFX::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[],
                                 const uint8_t mask[], int16_t w, int16_t h) {
  int16_t bw = (w + 7) / 8;
  uint8_t b = 0;
  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        b <<= 1;
      else
        b = pgm_read_byte(&mask[j * bw + i / 8]);
      if (b & 0x80) writePixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
    }
  }
  endWrite();
}

void Adafruit_GFX::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask,
                                 int16_t w, int16_t h) {
  int16_t bw = (w + 7) / 8;
  uint8_t b = 0;
  startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        b <<= 1;
      else
        b = mask[j * bw + i / 8];
      if (b & 0x80) writePixel(x + i, y, bitmap[j * w + i]);
    }
  }
  endWrite();
}

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
                            uint16_t color, uint16_t bg, uint8_t size) {
  drawChar(x, y, c, color, bg, size, size);
}

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
                            uint16_t color, uint16_t bg, uint8_t size_x,
                            uint8_t size_y) {
  if ((x >= _width) || (y >= _height) || ((x + 6 * size_x - 1) < 0) ||
      ((y + 8 * size_y - 1) < 0))
    return;

  if (!_cp437 && (c >= 176))
    c++;

  startWrite();
  for (int8_t i = 0; i < 5; i++) {
    uint8_t line = pgm_read_byte(&font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        if (size_x == 1 && size_y == 1)
          writePixel(x + i, y + j, color);
        else
          writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y, color);
      } else if (bg != color) {
        if (size_x == 1 && size_y == 1)
          writePixel(x + i, y + j, bg);
        else
          writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
      }
    }
  }
  if (bg != color) {
    if (size_x == 1 && size_y == 1)
      writeFastVLine(x + 5, y, 8, bg);
    else
      writeFillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
  }
  endWrite();
}

size_t Adafruit_GFX::write(uint8_t c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y += textsize_y * 8;
  } else if (c != '\r') {
    if (wrap && ((cursor_x + textsize_x * 6) > _width)) {
      cursor_x = 0;
      cursor_y += textsize_y * 8;
    }
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
             textsize_y);
    cursor_x += textsize_x * 6;
  }
  return 1;
}

GFXcanvas16::GFXcanvas16(uint16_t w, uint16_t h) : Adafruit_GFX(w, h) {
  buffer = (uint16_t *)malloc(w * h * sizeof(uint16_t));
  if (buffer)
    memset(buffer, 0, w * h * sizeof(uint16_t));
}

GFXcanvas16::~GFXcanvas16() { free(buffer); }

void GFXcanvas16::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (buffer && x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
    buffer[y * WIDTH + x] = color;
}

void GFXcanvas16::fillScreen(uint16_t color) {
  if (!buffer)
    return;
  for (uint32_t i = 0; i < (uint32_t)WIDTH * HEIGHT; i++)
    buffer[i] = color;
}

void GFXcanvas16::drawFastRawVLine(int16_t x, int16_t y, int16_t h,
                                   uint16_t color) {
  if (x < 0 || h <= 0)
    return;
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (y + h > HEIGHT)
    h = HEIGHT - y;
  if (h <= 0)
    return;
  while (h--)
    buffer[y++ * WIDTH + x] = color;
}

void GFXcanvas16::drawFastRawHLine(int16_t x, int16_t y, int16_t w,
                                   uint16_t color) {
  if (y < 0 || w <= 0)
    return;
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (x + w > WIDTH)
    w = WIDTH - x;
  if (w <= 0)
    return;
  uint16_t *ptr = buffer + y * WIDTH + x;
  while (w--)
    *ptr++ = color;
}

void GFXcanvas16::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                uint16_t color) {
  if (x < 0 || h <= 0)
    return;
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (y + h > HEIGHT)
    h = HEIGHT - y;
  if (h <= 0)
    return;
  drawFastRawVLine(x, y, h, color);
}

void GFXcanvas16::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                uint16_t color) {
  if (y < 0 || w <= 0)
    return;
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (x + w > WIDTH)
    w = WIDTH - x;
  if (w <= 0)
    return;
  drawFastRawHLine(x, y, w, color);
}
