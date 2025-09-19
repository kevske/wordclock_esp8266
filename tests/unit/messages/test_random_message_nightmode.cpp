#include <cstdio>
#include <cstdint>

// Simple harness that mirrors the gating logic added to wordclock_esp8266.ino
// We do not include the whole sketch; we just validate the state machine behavior.

static bool nightMode = false;
static bool ledOff = false;
static bool randomMessageActive = false;

// Simulates the 1Hz random message check and continuation logic
// triggerNow: whether shouldDisplayRandomMessage(hours, minutes, seconds) returned true
static void tickRandomMessage(bool triggerNow) {
  // Start condition: obey nightMode and ledOff
  if (!randomMessageActive && !nightMode && !ledOff && triggerNow) {
    randomMessageActive = true; // displayRandomMessage(true)
  }

  // Continue/cancel condition
  if (randomMessageActive) {
    if (nightMode || ledOff) {
      // Cancel immediately and clear matrix (implicit here)
      randomMessageActive = false;
    } else {
      // In the real code, displayRandomMessage(false) will eventually return 1 to end.
      // For test purposes, we keep it running until cancelled or until caller ends it.
    }
  }
}

static int g_failures = 0;
#define EXPECT_TRUE(cond, msg) \
  do { if (!(cond)) { std::printf("[FAIL] %s\n", msg); ++g_failures; } else { std::printf("[ OK ] %s\n", msg); } } while(0)
#define EXPECT_FALSE(cond, msg) EXPECT_TRUE(!(cond), msg)

int main() {
  std::printf("Running random message night mode gating tests...\n");

  // Case 1: Night mode prevents starting a random message
  nightMode = true;
  ledOff = false;
  randomMessageActive = false;
  tickRandomMessage(/*triggerNow=*/true);
  EXPECT_FALSE(randomMessageActive, "night mode blocks starting random message");

  // Case 2: LED off prevents starting a random message
  nightMode = false;
  ledOff = true;
  randomMessageActive = false;
  tickRandomMessage(/*triggerNow=*/true);
  EXPECT_FALSE(randomMessageActive, "LED-off blocks starting random message");

  // Case 3: Message active gets cancelled when night mode becomes active
  nightMode = false;
  ledOff = false;
  randomMessageActive = false;
  tickRandomMessage(/*triggerNow=*/true); // starts
  EXPECT_TRUE(randomMessageActive, "message started when allowed");
  nightMode = true; // night mode kicks in
  tickRandomMessage(/*triggerNow=*/false); // continue tick should cancel
  EXPECT_FALSE(randomMessageActive, "night mode cancels active message");

  // Case 4: Message active gets cancelled when LED-off becomes active
  nightMode = false;
  ledOff = false;
  randomMessageActive = false;
  tickRandomMessage(/*triggerNow=*/true); // starts
  EXPECT_TRUE(randomMessageActive, "message started when allowed (LED-off case)");
  ledOff = true; // LED-off kicks in
  tickRandomMessage(/*triggerNow=*/false); // continue tick should cancel
  EXPECT_FALSE(randomMessageActive, "LED-off cancels active message");

  std::printf("Failures: %d\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}
