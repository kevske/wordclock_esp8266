#pragma once
#include <cstdint>
#include <cstring>
#include <queue>
#include <string>

class UDP {
public:
  virtual ~UDP() = default;
  virtual void begin(uint16_t) {}
  virtual void stop() {}
  virtual int parsePacket() {
    return incoming.empty() ? 0 : (int)incoming.front().size();
  }
  virtual void flush() {
    if (!incoming.empty()) incoming.pop();
  }
  virtual int read(uint8_t* buffer, size_t len) {
    if (incoming.empty()) return 0;
    auto& pkt = incoming.front();
    size_t n = pkt.size() < len ? pkt.size() : len;
    std::memcpy(buffer, pkt.data(), n);
    incoming.pop();
    return (int)n;
  }
  virtual void beginPacket(const char*, uint16_t) { /* ignore */ }
  virtual void beginPacket(uint32_t, uint16_t) { /* ignore */ }
  virtual size_t write(const uint8_t*, size_t) { return 0; }
  virtual void endPacket() {}

  // Test helper: enqueue a packet to be "received"
  void enqueuePacket(const uint8_t* data, size_t len) {
    incoming.emplace(std::string(reinterpret_cast<const char*>(data), len));
  }

protected:
  std::queue<std::string> incoming;
};

class WiFiUDP : public UDP {
public:
  using UDP::UDP;
};
