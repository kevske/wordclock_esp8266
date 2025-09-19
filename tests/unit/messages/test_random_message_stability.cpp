#include <cstdio>
#include <cstdint>
#include <vector>

// This is a lightweight model of the random message progression.
// We simulate a path of fixed length and ensure that previously lit
// positions remain on while we advance through the message.

static const int kPathLen = 12; // arbitrary test length
static std::vector<bool> active(kPathLen, false);
static int currentIdx = 0;

// Advances the message by one step. Emulates the code's behavior of
// re-drawing all active LEDs, not clearing previous ones.
static void step_message() {
  if (currentIdx < kPathLen) {
    active[currentIdx] = true; // turn on the next LED
    // Emulate redraw: ensure we do NOT turn off previous ones.
    // No-op here because 'active' holds state; the important part is
    // that we never set earlier entries to false.
    currentIdx++;
  }
}

static int g_failures = 0;
#define EXPECT_TRUE(cond, msg) \
  do { if (!(cond)) { std::printf("[FAIL] %s\n", msg); ++g_failures; } else { std::printf("[ OK ] %s\n", msg); } } while(0)

int main() {
  std::printf("Running random message stability test (cumulative letters)...\n");

  // Start: nothing is active
  for (int i = 0; i < kPathLen; ++i) EXPECT_TRUE(!active[i], "initially off");

  // After first step, index 0 should be on
  step_message();
  EXPECT_TRUE(active[0], "first LED turns on");

  // After second step, indices 0 and 1 should be on
  step_message();
  EXPECT_TRUE(active[0], "LED 0 stays on after step 2");
  EXPECT_TRUE(active[1], "LED 1 turns on at step 2");

  // Advance a few more steps and ensure cumulative behavior
  for (int s = 0; s < 5; ++s) step_message();
  // We have executed 7 steps in total; indices 0..6 should be on
  for (int i = 0; i < 7; ++i) {
    char msg[64];
    std::snprintf(msg, sizeof(msg), "LED %d remains on cumulatively", i);
    EXPECT_TRUE(active[i], msg);
  }

  // And indices >= 7 should still be off at this point
  for (int i = 7; i < kPathLen; ++i) {
    char msg[64];
    std::snprintf(msg, sizeof(msg), "LED %d remains off until reached", i);
    EXPECT_TRUE(!active[i], msg);
  }

  std::printf("Failures: %d\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}
