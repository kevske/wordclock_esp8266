#pragma once
#include <cstdint>

class Adafruit_NeoMatrix {
public:
  inline void begin() {}
  inline void setTextWrap(bool) {}
  inline void setBrightness(uint8_t) {}
  inline void drawPixel(int, int, uint16_t) {}
  inline void show() {}
};
