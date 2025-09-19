#pragma once
#include <cstdint>
#include <cstdio>

using byte = uint8_t;

// Simulated time base for tests
static unsigned long __mock_millis = 0;

inline void randomSeed(unsigned long) {}
inline int analogRead(uint8_t) { return 0; }

inline unsigned long millis() { return __mock_millis; }
inline void delay(unsigned long ms) { __mock_millis += ms; }

#ifndef LOW
#define LOW 0
#endif
#ifndef HIGH
#define HIGH 1
#endif

// Arduino word() helper
inline unsigned int word(uint8_t high, uint8_t low) {
  return ((unsigned int)high << 8) | low;
}

// Minimal String stand-in if needed in future (avoid pulling std::string everywhere)
class String {
public:
  String() = default;
  String(const char*) {}
  String(const String&) = default;
  // Simple helpers used in code under test
  String operator+(const String&) const { return String(""); }
  String operator+(const char*) const { return String(""); }
  String operator+(int) const { return String(""); }
  const char* c_str() const { return ""; }
};
