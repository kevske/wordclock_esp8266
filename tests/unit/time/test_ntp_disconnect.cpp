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
  // Fill the 32-bit transmit timestamp seconds part into bytes 40..43
  unsigned long highWord = (seconds_since_1900 >> 16) & 0xFFFF;
  unsigned long lowWord  = (seconds_since_1900 & 0xFFFF);
  buf[40] = (highWord >> 8) & 0xFF;
  buf[41] = (highWord) & 0xFF;
  buf[42] = (lowWord >> 8) & 0xFF;
  buf[43] = (lowWord) & 0xFF;
}

int main() {
  std::printf("Running NTP disconnect tests...\n");

  WiFiUDP udp;
  int utc_minutes = 0; // UTC
  NTPClientPlus ntp(udp, "pool.ntp.org", utc_minutes, true);

  // Seed one successful NTP update: choose 1970-01-01 01:00:00 UTC
  const unsigned long EPOCH_OFFSET_1900_TO_1970 = 2208988800UL;
  unsigned long t_seed = EPOCH_OFFSET_1900_TO_1970 + 3600UL; // 1 hour after 1970 epoch
  uint8_t pkt[NTP_PACKET_SIZE];
  makeNtpPacket(pkt, t_seed);

  udp.begin(1337);

  // Ensure no stale packets are flushed before we try: queue AFTER any potential flushes.
  // Directly call updateNTP (without setupNTPClient) so we control the queue timing.
  udp.enqueuePacket(pkt, sizeof(pkt));
  int r = ntp.updateNTP();
  EXPECT_EQ(r, 0, "initial NTP update succeeds");

  unsigned long t0 = ntp.getEpochTime();
  EXPECT_TRUE(t0 >= 3600UL && t0 < 3605UL, "epoch near seeded time");

  // Advance 5 seconds of mocked time without any incoming packets
  delay(5000);

  // Simulate a failed update due to timeout (no packet enqueued)
  int r_fail = ntp.updateNTP();
  EXPECT_EQ(r_fail, -1, "updateNTP times out when no packet is received");

  unsigned long t1 = ntp.getEpochTime();
  EXPECT_TRUE(t1 >= t0 + 5, "epoch continues to advance after failed update");

  // Advance another 30s and repeat failure
  delay(30000);
  r_fail = ntp.updateNTP();
  EXPECT_EQ(r_fail, -1, "second timeout");
  unsigned long t2 = ntp.getEpochTime();
  EXPECT_TRUE(t2 >= t1 + 30, "epoch keeps advancing across multiple failures");

  std::printf("Failures: %d\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}
