#include <cstdio>
#include <cstdint>
#include <cmath>
#include <string>

// Include mocks first so they override real headers
#include "../mocks/Arduino.h"
#include "../mocks/Adafruit_GFX.h"
#include "../mocks/Adafruit_NeoMatrix.h"

// Include the code under test
#include "../../../ledmatrix.h"

static int g_failures = 0;

#define EXPECT_EQ(actual, expected, msg) \
  do { \
    auto a = (actual); \
    auto e = (expected); \
    if (a != e) { \
      std::printf("[FAIL] %s: got=0x%08X expected=0x%08X\n", msg, (unsigned)(a), (unsigned)(e)); \
      ++g_failures; \
    } else { \
      std::printf("[ OK ] %s\n", msg); \
    } \
  } while (0)

static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

int main() {
  std::printf("Running LEDMatrix color math tests...\n");

  // Color24bit basic packing
  EXPECT_EQ(LEDMatrix::Color24bit(0,0,0), rgb(0,0,0), "Color24bit black");
  EXPECT_EQ(LEDMatrix::Color24bit(255,255,255), rgb(255,255,255), "Color24bit white");
  EXPECT_EQ(LEDMatrix::Color24bit(0x12,0x34,0x56), rgb(0x12,0x34,0x56), "Color24bit 0x123456");

  // color24to16bit conversion (RGB888 -> RGB565)
  EXPECT_EQ(LEDMatrix::color24to16bit(rgb(255,0,0)), (uint16_t)0xF800, "RGB565 red");
  EXPECT_EQ(LEDMatrix::color24to16bit(rgb(0,255,0)), (uint16_t)0x07E0, "RGB565 green");
  EXPECT_EQ(LEDMatrix::color24to16bit(rgb(0,0,255)), (uint16_t)0x001F, "RGB565 blue");
  EXPECT_EQ(LEDMatrix::color24to16bit(rgb(255,255,255)), (uint16_t)0xFFFF, "RGB565 white");
  EXPECT_EQ(LEDMatrix::color24to16bit(rgb(0,0,0)), (uint16_t)0x0000, "RGB565 black");

  // Wheel should return values in the RGB space – spot check endpoints
  EXPECT_EQ(LEDMatrix::Wheel(0), LEDMatrix::Color24bit(255,0,0), "Wheel(0) red end");
  EXPECT_EQ(LEDMatrix::Wheel(85), LEDMatrix::Color24bit(0,255,0), "Wheel(85) green end");
  EXPECT_EQ(LEDMatrix::Wheel(170), LEDMatrix::Color24bit(0,0,255), "Wheel(170) blue end");

  // Interpolation mid-point check
  uint32_t c1 = rgb(0,0,0);
  uint32_t c2 = rgb(255, 127, 63);
  uint32_t mid = LEDMatrix::interpolateColor24bit(c1, c2, 0.5f);
  EXPECT_EQ(mid, rgb(127, 63, 31), "interpolate 50% from black to (255,127,63)");

  std::printf("Failures: %d\n", g_failures);
  return (g_failures == 0) ? 0 : 1;
}
