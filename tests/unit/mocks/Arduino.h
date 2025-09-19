#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

using byte = uint8_t;

// Global simulated time base for tests (defined in Arduino_time.cpp)
extern unsigned long __mock_millis;

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

// Minimal String stand-in for tests
class String {
public:
  String() = default;
  String(const char* s) : data_(s ? s : "") {}
  String(const std::string& s) : data_(s) {}
  String(const String&) = default;
  String& operator=(const String&) = default;

  // Numeric constructors used in code under test
  String(int v) : data_(std::to_string(v)) {}
  String(unsigned int v) : data_(std::to_string(v)) {}
  String(long v) : data_(std::to_string(v)) {}
  String(unsigned long v) : data_(std::to_string(v)) {}

  // Concatenation operators
  String operator+(const String& rhs) const { return String(data_ + rhs.data_); }
  String operator+(const char* rhs) const { return String(data_ + (rhs ? std::string(rhs) : std::string())); }
  String operator+(int rhs) const { return String(data_ + std::to_string(rhs)); }

  // Access
  const char* c_str() const { return data_.c_str(); }

private:
  std::string data_;
};

// Support "literal" + String concatenation
inline String operator+(const char* lhs, const String& rhs) {
  return String(std::string(lhs ? lhs : "") + rhs.c_str());
}
