// Test translation unit for LEDMatrix that forces mocks and avoids real UDPLogger header
// Inject a guard so ledmatrix.h's "#include \"udplogger.h\"" is skipped
#ifndef udplogger_h
#define udplogger_h
class UDPLogger; // forward declaration is sufficient for pointers
#endif

// Provide mocks for Arduino/Adafruit
#include "../mocks/Arduino.h"
#include "../mocks/Adafruit_GFX.h"
#include "../mocks/Adafruit_NeoMatrix.h"

// Include the real implementation after mocks and stub guard
#include "../../../ledmatrix.cpp"
