#include "weather_client.h"

WeatherClient::WeatherClient() {
  tempTodayNoon = 0;
  tempTomorrowNoon = 0;
  sunshineToday = 0;
  sunshineTomorrow = 0;
  lastUpdate = 0;
  lastAttempt = 0;
  dataValid = false;
}

void WeatherClient::update() {
  // Skip if WiFi is not connected - avoids blocking connection attempts
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  // Use cached data if still valid (30 minutes)
  if (dataValid && (millis() - lastUpdate < 1800000)) {
    return;
  }

  // Rate-limit connection attempts: at most once every 60 seconds when failing
  if (!dataValid && (millis() - lastAttempt < 60000)) {
    return;
  }
  lastAttempt = millis();

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation for simplicity/speed on ESP8266
  client.setTimeout(5000);  // 5 second timeout (default is ~30s)

  if (!client.connect(host, 443)) {
    Serial.println("WeatherClient: Connection failed");
    return;
  }

  // Send HTTP request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266WordClock\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  // Read response
  String payload = client.readString();
  parseJson(payload);
  lastUpdate = millis();
}

void WeatherClient::parseJson(String payload) {
  // "hourly": { "time": [...], "temperature_2m": [ ... ] }
  // We need index 12 (12:00 Today) and index 36 (12:00 Tomorrow)
  // Indices are 0-based from 00:00 today.
  
  // NOTE: This manual parser is fragile but sufficient for this specific structure if Open-Meteo doesn't change format drastically.
  // It looks for "temperature_2m" and then parses the array.

  // Use simple keys and rely on robust parsing to find the array
  // Use "\"temperature_2m\"" (etc) to match the key regardless of following spaces or structure
  tempTodayNoon = extractValueAtIndex(payload, "\"temperature_2m\"", 12);
  tempTomorrowNoon = extractValueAtIndex(payload, "\"temperature_2m\"", 36);
  
  // Extract weather codes
  codeTodayNoon = (int)extractValueAtIndex(payload, "\"weathercode\"", 12);
  codeTomorrowNoon = (int)extractValueAtIndex(payload, "\"weathercode\"", 36);
  
  // Extract sunshine duration (daily array)
  sunshineToday = extractDailyValueAtIndex(payload, "\"sunshine_duration\"", 0);
  sunshineTomorrow = extractDailyValueAtIndex(payload, "\"sunshine_duration\"", 1);
  
  // Basic validation check (e.g. -50 to +50 is reasonable range)
  if (tempTodayNoon > -60 && tempTodayNoon < 60) {
     dataValid = true;
     Serial.printf("Weather Update: Today Noon: %.1f C, Sun: %.1f h | Tomorrow Noon: %.1f C, Sun: %.1f h\n", 
                    tempTodayNoon, sunshineToday/3600.0, tempTomorrowNoon, sunshineTomorrow/3600.0);
  } else {
     dataValid = false;
  }
}

// Extracts the Nth value from a JSON array in the "daily" section
// This is needed because "sunshine_duration" appears twice in the API response:
// once in "daily_units" (as a string "s") and once in "daily" (as the data array)
float WeatherClient::extractDailyValueAtIndex(String payload, String key, int targetIndex) {
  // First, find the "daily":{ section (not "daily_units")
  int dailyStart = payload.indexOf("\"daily\":{");
  if (dailyStart == -1) {
    // Try alternate format with space
    dailyStart = payload.indexOf("\"daily\": {");
  }
  if (dailyStart == -1) return -999;
  
  // Now search for the key starting from within the daily section
  int keyPos = payload.indexOf(key, dailyStart);
  if (keyPos == -1) return -999;
  
  // Find the array start
  int cursor = keyPos + key.length();
  while (cursor < payload.length() && (payload[cursor] == ' ' || payload[cursor] == ':')) {
    cursor++;
  }
  
  if (cursor >= payload.length() || payload[cursor] != '[') return -999;
  
  int arrayStart = cursor;
  int currentIndex = 0;
  int searchPos = arrayStart + 1;
  
  // Skip to the target index
  while (currentIndex < targetIndex) {
    int commaPos = payload.indexOf(',', searchPos);
    if (commaPos == -1) return -999;
    searchPos = commaPos + 1;
    currentIndex++;
  }
  
  // Extract the number
  int nextComma = payload.indexOf(',', searchPos);
  int arrayEnd = payload.indexOf(']', searchPos);
  
  int endPos;
  if (nextComma == -1) endPos = arrayEnd;
  else if (arrayEnd == -1) endPos = nextComma;
  else endPos = (nextComma < arrayEnd) ? nextComma : arrayEnd;
  
  if (endPos == -1) return -999;
  
  String valStr = payload.substring(searchPos, endPos);
  return valStr.toFloat();
}

float WeatherClient::extractValueAtIndex(String payload, String key, int targetIndex) {
  int startSearch = 0;
  int arrayStart = -1;
  
  // Loop to find the key followed by a '[' (checking for valid data array vs metadata)
  while (true) {
    int keyPos = payload.indexOf(key, startSearch);
    if (keyPos == -1) return -999;
    
    // Check what follows
    int cursor = keyPos + key.length();
    // Skip whitespace and colon
    while (cursor < payload.length() && (payload[cursor] == ' ' || payload[cursor] == '\t' || payload[cursor] == '\r' || payload[cursor] == '\n' || payload[cursor] == ':')) {
       cursor++;
    }

    if (cursor < payload.length() && payload[cursor] == '[') {
       // Found it!
       arrayStart = cursor;
       break; 
    } else {
       // Not the one (e.g. was units), search next
       startSearch = keyPos + 1;
    }
  }

  int currentIndex = 0;
  int searchPos = arrayStart + 1;
  
  while (currentIndex < targetIndex) {
    int commaPos = payload.indexOf(',', searchPos);
    if (commaPos == -1) return -999; // End of array before index
    searchPos = commaPos + 1;
    currentIndex++;
  }

  // extract number
  int nextComma = payload.indexOf(',', searchPos);
  int arrayEnd = payload.indexOf(']', searchPos);
  
  int endPos;
  if (nextComma == -1) endPos = arrayEnd;
  else if (arrayEnd == -1) endPos = nextComma;
  else endPos = (nextComma < arrayEnd) ? nextComma : arrayEnd;
  
  if (endPos == -1) return -999;

  String valDir = payload.substring(searchPos, endPos);
  return valDir.toFloat();
}

float WeatherClient::getSunshineDuration(bool tomorrow) {
    if (tomorrow) return sunshineTomorrow;
    return sunshineToday;
}

int WeatherClient::getTemperature(bool tomorrow) {
  if (tomorrow) return (int)round(tempTomorrowNoon);
  return (int)round(tempTodayNoon);
}

int WeatherClient::getWeatherCode(bool tomorrow) {
    if (tomorrow) return codeTomorrowNoon;
    return codeTodayNoon;
}

bool WeatherClient::isDataValid() {
    return dataValid;
}

void WeatherClient::invalidateCache() {
    dataValid = false;
}
