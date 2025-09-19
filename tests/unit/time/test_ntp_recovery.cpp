#include <cstdio>
#include <cstdint>
#include <cstring>

#include "../mocks/Arduino.h"
#include "../mocks/WiFiUdp.h"

#include "../../../ntp_client_plus.h"

static int g_failures = 0;
#define EXPECT_TRUE(cond, msg) \
  do { if (!(cond)) { std::printf("[FAIL] %s\n", msg); ++g_failures; } else { std::printf("[ OK ] %s\n", msg); } } while(0)
#define EXPECT_EQ(a,b,msg) \
  do { auto _a=(a); auto _b=(b); if (_a!=_b) { std::printf("[FAIL] %s got=%lu exp=%lu\n", msg, (unsigned long)_a, (unsigned long)_b); ++g_failures; } else { std::printf("[ OK ] %s\n", msg); } } while(0)

static void makeNtpPacket(uint8_t* buf, unsigned long seconds_since_1900) {
  std::memset(buf, 0, NTP_PACKET_SIZE);
  unsigned long highWord = (seconds_since_1900 >> 16) & 0xFFFF;
  unsigned long lowWord  = (seconds_since_1900 & 0xFFFF);
  buf[40] = (highWord >> 8) & 0xFF;
  buf[41] = (highWord) & 0xFF;
  buf[42] = (lowWord >> 8) & 0xFF;
  buf[43] = (lowWord) & 0xFF;
}

int main() {
  std::printf("Running NTP recovery tests...\n");

  // Ensure clean mock time
  __mock_millis = 0;

  WiFiUDP udp;
  int utc_minutes = 0; // UTC
  NTPClientPlus ntp(udp, "pool.ntp.org", utc_minutes, true);

  const unsigned long EPOCH_OFFSET_1900_TO_1970 = 2208988800UL;
  unsigned long t_seed = EPOCH_OFFSET_1900_TO_1970 + 7200UL; // 2h after epoch
  uint8_t pkt[NTP_PACKET_SIZE];
  makeNtpPacket(pkt, t_seed);

  udp.begin(1337);
  // Prepare the response so updateNTP()'s endPacket() will deliver it
  udp.prepareIncoming(pkt, sizeof(pkt));
  int r = ntp.updateNTP();
  EXPECT_EQ(r, 0, "seed update ok");

  unsigned long base = ntp.getEpochTime();
  EXPECT_TRUE(base >= 7200UL && base < 7205UL, "epoch near 2h");

  // Simulate two failed updates across 20s of time
  delay(10000);
  EXPECT_EQ(ntp.updateNTP(), -1, "timeout #1");
  delay(10000);
  EXPECT_EQ(ntp.updateNTP(), -1, "timeout #2");
  unsigned long beforeRecover = ntp.getEpochTime();
  EXPECT_TRUE(beforeRecover >= base + 20, "time advanced across failures");

  // Now deliver a fresh NTP packet 1 hour later than seed
  unsigned long t_recover = EPOCH_OFFSET_1900_TO_1970 + 10800UL; // 3h after epoch
  makeNtpPacket(pkt, t_recover);
  udp.prepareIncoming(pkt, sizeof(pkt));
  int rr = ntp.updateNTP();
  EXPECT_EQ(rr, 0, "recovery update ok");

  unsigned long afterRecover = ntp.getEpochTime();
  // Should be close to 3h, minus any in-flight delays accounted internally
  EXPECT_TRUE(afterRecover >= 10800UL && afterRecover < 10810UL, "epoch jumps to recovered time");

  std::printf("Failures: %d\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}
