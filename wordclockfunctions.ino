const String clockStringGerman =  "ESHISTPZEHNHALBZWANZIGRNFUNFQKJMSATVIERTELAASONACHXVORRSNTCDREIBLANISIEBENLEHEACHTELFUNFRZWOLFUVIERSECHSZWEINSZEHNEUNKUHR";

// Lookup table for minute indicator patterns: maps (minutes % 5) to LED pattern
// Pattern is binary: 0b1000 = first LED, 0b1100 = first two, etc.
static const uint8_t minutePatterns[5] = {0b0000, 0b1000, 0b1100, 0b1110, 0b1111};
static const uint8_t singleLedPatterns[5] = {0b0000, 0b1000, 0b0100, 0b0010, 0b0001};

/**
 * @brief Helper function for special time range minute indicators (25-30, 35-40)
 * Sets all LEDs to base color, then highlights one LED in white based on minutes
 * 
 * @param minutes current minutes value
 * @param baseColor background color for all 4 LEDs
 * @param highlightColor color for the highlighted LED (typically white)
 */
void setSpecialMinuteIndicator(uint8_t minutes, uint32_t baseColor, uint32_t highlightColor) {
  // Set all LEDs to the base color
  ledmatrix.setMinIndicator(0b1111, baseColor);
  
  // Highlight the appropriate LED based on minutes % 5
  uint8_t minuteMod = minutes % 5;
  if (minuteMod > 0) {
    ledmatrix.setMinIndicator(singleLedPatterns[minuteMod], highlightColor);
  }
}

/**
 * @brief control the four minute indicator LEDs
 * 
 * @param minutes minutes to be displayed [0 ... 59]
 * @param color 24bit color value
 */
void drawMinuteIndicator(uint8_t minutes, uint32_t color){
  // Define colors for special conditions
  const uint32_t greenColor = 0x00FF00;
  const uint32_t redColor = 0xFF0000;
  const uint32_t whiteColor = 0xFFFFFF;

  // Special time ranges with colored backgrounds
  if (minutes >= 25 && minutes < 30) {
    // Green background with white highlight for 25-29 minutes
    setSpecialMinuteIndicator(minutes, greenColor, whiteColor);
  }
  else if (minutes >= 35 && minutes < 40) {
    // Red background with white highlight for 35-39 minutes
    setSpecialMinuteIndicator(minutes, redColor, whiteColor);
  }
  else {
    // Normal behavior: progressive LED fill based on minutes % 5
    uint8_t minuteMod = minutes % 5;
    if (minuteMod > 0) {
      ledmatrix.setMinIndicator(minutePatterns[minuteMod], color);
    }
  }
}

/**
 * @brief Display a random message on the LED matrix
 * 
 * This function displays one of 4 possible messages on the LED matrix.
 * The messages always start with the third and fourth LED horizontally on the top row
 * and end with the third to ninth LED vertically in the first column.
 * The message is displayed in red for 40 seconds, with LEDs turning on one by one with a 0.5 second delay.
 * 
 * @param init If true, starts displaying a new message; if false, continues displaying or turns off
 * @return int - 1 when message display is finished, 0 otherwise
 */
int displayRandomMessage(bool init) {
  // Define message paths (coordinates for LEDs to light up)
  static const uint8_t messagePaths[4][20][2] = {
    { // Message 1: Second to eighth LED vertically in the second column
      {2, 0}, {3, 0},                  // Start: third and fourth LED horizontally on top row
      {1, 1}, {1, 2}, {1, 3}, {1, 4},  // Second column: second to eighth LED vertically
      {1, 5}, {1, 6}, {1, 7},
      {0, 2}, {0, 3}, {0, 4}, {0, 5},  // End: third to ninth LED vertically in first column
      {0, 6}, {0, 7}, {0, 8}
    },
    { // Message 2: Third to ninth LED vertically in the tenth column
      {2, 0}, {3, 0},                  // Start: third and fourth LED horizontally on top row
      {9, 2}, {9, 3}, {9, 4}, {9, 5},  // Tenth column: third to ninth LED vertically
      {9, 6}, {9, 7}, {9, 8},
      {0, 2}, {0, 3}, {0, 4}, {0, 5},  // End: third to ninth LED vertically in first column
      {0, 6}, {0, 7}, {0, 8}
    },
    { // Message 3: Third to seventh LED vertically in the eleventh column
      {2, 0}, {3, 0},                  // Start: third and fourth LED horizontally on top row
      {10, 2}, {10, 3}, {10, 4},       // Eleventh column: third to seventh LED vertically
      {10, 5}, {10, 6},
      {0, 2}, {0, 3}, {0, 4}, {0, 5},  // End: third to ninth LED vertically in first column
      {0, 6}, {0, 7}, {0, 8}
    },
    { // Message 4: Third to seventh LED vertically in the eighth column
      {2, 0}, {3, 0},                  // Start: third and fourth LED horizontally on top row
      {7, 2}, {7, 3}, {7, 4},          // Eighth column: third to seventh LED vertically
      {7, 5}, {7, 6},
      {0, 2}, {0, 3}, {0, 4}, {0, 5},  // End: third to ninth LED vertically in first column
      {0, 6}, {0, 7}, {0, 8}
    }
  };

  // Number of LEDs in each message path
  static const uint8_t messageLengths[4] = {16, 16, 14, 14};
  
  // Static variables to track state between function calls
  static uint8_t currentMessage = 0;    // Current message being displayed (0-3)
  static uint8_t currentLed = 0;        // Current LED being turned on
  static unsigned long startTime = 0;   // When the message started displaying
  static unsigned long lastLedTime = 0; // When the last LED was turned on
  static bool isDisplaying = false;     // Whether a message is currently being displayed
  static bool isClearing = false;       // Whether we're in the process of clearing the message
  static uint32_t messageColor = 0xFF0000; // Red color for the message
  static uint8_t savedCoords[20][2];    // To save the coordinates (x,y) of the message LEDs
  static bool activeLeds[20] = {false}; // Track which LEDs are currently active

  // If initializing, select a random message and start displaying
  if (init) {
    currentMessage = random(4);  // Select a random message (0-3)
    currentLed = 0;
    startTime = millis();
    lastLedTime = startTime;
    isDisplaying = true;
    isClearing = false;
    
    // Reset active LEDs tracking
    for (uint8_t i = 0; i < 20; i++) {
      activeLeds[i] = false;
    }
    
    // Save the current state of LEDs that will be part of the message
    for (uint8_t i = 0; i < messageLengths[currentMessage]; i++) {
      uint8_t x = messagePaths[currentMessage][i][0];
      uint8_t y = messagePaths[currentMessage][i][1];
      
      // Save the current color at this position (we'll need to restore it later)
      // We're using the targetgrid directly, which isn't ideal but necessary to preserve state
      savedCoords[i][0] = x;
      savedCoords[i][1] = y;
    }
    
    logger.logString("Starting random message display: " + String(currentMessage));
    return 0;
  }

  // If not displaying a message, return immediately
  if (!isDisplaying) {
    return 1;
  }

  unsigned long currentTime = millis();

  // Phase 1: Turn on LEDs one by one with delay
  if (!isClearing && currentLed < messageLengths[currentMessage]) {
    if (currentTime - lastLedTime >= 500) { // 0.5 second delay between LEDs
      // Mark this LED as active
      activeLeds[currentLed] = true;
      
      // Redraw all active LEDs
      for (uint8_t i = 0; i < messageLengths[currentMessage]; i++) {
        if (activeLeds[i]) {
          uint8_t x = messagePaths[currentMessage][i][0];
          uint8_t y = messagePaths[currentMessage][i][1];
          
          // Turn on this LED in red
          ledmatrix.gridAddPixel(x, y, messageColor);
        }
      }
      
      lastLedTime = currentTime;
      currentLed++;
      
      // Update the display
      ledmatrix.drawOnMatrixInstant();
    }
    return 0;
  }
  
  // Phase 2: Keep message displayed for 40 seconds
  if (!isClearing && currentTime - startTime < 40000) {
    // Ensure all active LEDs remain on (in case they were somehow turned off)
    for (uint8_t i = 0; i < messageLengths[currentMessage]; i++) {
      if (activeLeds[i]) {
        uint8_t x = messagePaths[currentMessage][i][0];
        uint8_t y = messagePaths[currentMessage][i][1];
        
        // Turn on this LED in red
        ledmatrix.gridAddPixel(x, y, messageColor);
      }
    }
    
    // Update the display periodically to ensure LEDs stay on
    if (currentTime - lastLedTime >= 1000) { // Check every second
      ledmatrix.drawOnMatrixInstant();
      lastLedTime = currentTime;
    }
    
    return 0;
  }
  
  // Phase 3: Clear the message (turn off all LEDs that were part of the message)
  if (!isClearing && currentTime - startTime >= 40000) {
    isClearing = true;
    currentLed = 0;
    lastLedTime = currentTime;
    logger.logString("Clearing random message");
    return 0;
  }
  
  // Turn off LEDs one by one
  if (isClearing && currentLed < messageLengths[currentMessage]) {
    if (currentTime - lastLedTime >= 100) { // Faster clearing (0.1 second delay)
      // Mark this LED as inactive
      activeLeds[currentLed] = false;
      
      uint8_t x = savedCoords[currentLed][0];
      uint8_t y = savedCoords[currentLed][1];
      
      // Reset this LED to its original state by redrawing the current time
      // We need to clear this specific LED without affecting others
      ledmatrix.gridAddPixel(x, y, 0); // Turn off this LED
      
      lastLedTime = currentTime;
      currentLed++;
      
      // Update the display
      ledmatrix.drawOnMatrixInstant();
    }
    return 0;
  }
  
  // Message display complete
  if (isClearing && currentLed >= messageLengths[currentMessage]) {
    isDisplaying = false;
    isClearing = false;
    logger.logString("Random message display complete");
    return 1;
  }
  
  return 0;
}

/**
 * @brief Check if it's time to display a random message (at a random time each hour)
 * 
 * @param hours Current hour
 * @param minutes Current minute
 * @param seconds Current second
 * @return bool - true if a new message should be displayed
 */
bool shouldDisplayRandomMessage(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  static uint8_t lastHour = 255; // Initialize to invalid hour
  static uint8_t targetMinute = 0;
  
  // If the hour has changed, set a new random minute for this hour
  if (hours != lastHour) {
    targetMinute = random(0, 60); // Random minute in the hour
    lastHour = hours;
    logger.logString("New random message time set: " + String(hours) + ":" + String(targetMinute));
    return false;
  }
  
  // Check if we've reached the target minute and the seconds are 0
  // This ensures we only trigger once per hour at the exact minute
  if (minutes == targetMinute && seconds == 0) {
    return true;
  }
  
  return false;
}

/**
 * @brief Draw the given sentence to the word clock
 * 
 * @param message sentence to be displayed
 * @param color 24bit color value
 * @return int: 0 if successful, -1 if sentence not possible to display
 */
int showStringOnClock(String message, uint32_t color){
    String word = "";
    int lastLetterClock = 0;
    int positionOfWord  = 0;
    int index = 0;

    // add space on the end of message for splitting
    message = message + " ";

    // empty the targetgrid
    ledmatrix.gridFlush();

    while(true){
      // extract next word from message
      word = split(message, ' ', index);
      index++;
      
      if(word.length() > 0){
        // find word in clock string
        positionOfWord = clockStringGerman.indexOf(word, lastLetterClock);
        
        if(positionOfWord >= 0){
          // word found on clock -> enable leds in targetgrid
          for(unsigned int i = 0; i < word.length(); i++){
            int x = (positionOfWord + i)%WIDTH;
            int y = (positionOfWord + i)/WIDTH;
            ledmatrix.gridAddPixel(x, y, color);
          }
          // remember end of the word on clock
          lastLetterClock = positionOfWord + word.length();
        }
        else{
          // word is not possible to show on clock
          logger.logString("word is not possible to show on clock: " + String(word));
          return -1;
        }
      }else{
        // end - no more word in message
        break;
      }
    }
    // return success
    return 0;
}

/**
 * @brief Converts the given time as sentence (String)
 * 
 * @param hours hours of the time value
 * @param minutes minutes of the time value
 * @return String time as sentence
 */
String timeToString(uint8_t hours,uint8_t minutes){
  
  //ES IST
  String message;\r\n  message.reserve(50); // Pre-allocate to avoid heap fragmentation\r\n  message = "ES IST ";

  
  //show minutes
  if(minutes >= 5 && minutes < 10)
  {
    message += "FUNF NACH ";
  }
  else if(minutes >= 10 && minutes < 15)
  {
    message += "ZEHN NACH ";
  }
  else if(minutes >= 15 && minutes < 20)
  {
    message += "VIERTEL NACH ";
  }
  else if(minutes >= 20 && minutes < 25)
  {
    message += "ZWANZIG NACH "; 
  }
  else if(minutes >= 25 && minutes < 30)
  {
    message += "HALB ";
  }
  else if(minutes >= 30 && minutes < 35)
  {
    message += "HALB ";
  }
  else if(minutes >= 35 && minutes < 40)
  {
    message += "HALB ";
  }
  else if(minutes >= 40 && minutes < 45)
  {
    message += "ZWANZIG VOR ";
  }
  else if(minutes >= 45 && minutes < 50)
  {
    message += "VIERTEL VOR ";
  }
  else if(minutes >= 50 && minutes < 55)
  {
    message += "ZEHN VOR ";
  }
  else if(minutes >= 55 && minutes < 60)
  {
    message += "FUNF VOR ";
  }

  //convert hours to 12h format
  if(hours >= 12)
  {
      hours -= 12;
  }
  if(minutes >= 25)
  {
      hours++;
  }
  if(hours == 12)
  {
      hours = 0;
  }

  // show hours
  switch(hours)
  {
  case 0:
    message += "ZWOLF ";
    break;
  case 1:
    message += "EIN";
    //EIN(S)
    if(minutes > 4){
      message += "S";
    }
    message += " ";
    break;
  case 2:
    message += "ZWEI ";
    break;
  case 3:
    message += "DREI ";
    break;
  case 4:
    message += "VIER ";
    break;
  case 5:
    message += "FUNF ";
    break;
  case 6:
    message += "SECHS ";
    break;
  case 7:
    message += "SIEBEN ";
    break;
  case 8:
    message += "ACHT ";
    break;
  case 9:
    message += "NEUN ";
    break;
  case 10:
    message += "ZEHN ";
    break;
  case 11:
    message += "ELF ";
    break;
  }
  if(minutes < 5)
  {
    message += "UHR ";
  }

  logger.logString("time as String: " + String(message));

  return message;
}
