#include "weather_client.h"

WeatherClient::WeatherClient() {
  tempTodayNoon = 0;
  tempTomorrowNoon = 0;
  sunshineToday = 0;
  sunshineTomorrow = 0;
  lastUpdate = 0;
  dataValid = false;
}

void WeatherClient::update() {
  // Update every 30 minutes or if never updated
  if (dataValid && (millis() - lastUpdate < 1800000)) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation for simplicity/speed on ESP8266

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

  // Use ":[" to ensure we match the data array and not the hourly_units metadata
  tempTodayNoon = extractValueAtIndex(payload, "\"temperature_2m\":[", 12);
  tempTomorrowNoon = extractValueAtIndex(payload, "\"temperature_2m\":[", 36);
  
  // Extract weather codes
  codeTodayNoon = (int)extractValueAtIndex(payload, "\"weathercode\":[", 12);
  codeTomorrowNoon = (int)extractValueAtIndex(payload, "\"weathercode\":[", 36);
  
  // Extract sunshine duration (daily array)
  sunshineToday = extractDailyValueAtIndex(payload, "\"sunshine_duration\":[", 0);
  sunshineTomorrow = extractDailyValueAtIndex(payload, "\"sunshine_duration\":[", 1);
  
  // Basic validation check (e.g. -50 to +50 is reasonable range)
  if (tempTodayNoon > -60 && tempTodayNoon < 60) {
     dataValid = true;
     Serial.printf("Weather Update: Today Noon: %.1f C, Sun: %.1f h | Tomorrow Noon: %.1f C, Sun: %.1f h\n", 
                    tempTodayNoon, sunshineToday/3600.0, tempTomorrowNoon, sunshineTomorrow/3600.0);
  } else {
     dataValid = false;
  }
}

// Extracts the Nth value from a JSON array identified by key (Same logic, helper to keep code clean)
float WeatherClient::extractDailyValueAtIndex(String payload, String key, int targetIndex) {
  // exact same implementation as extractValueAtIndex for now, just reused logic
  return extractValueAtIndex(payload, key, targetIndex);
}

float WeatherClient::extractValueAtIndex(String payload, String key, int targetIndex) {
  int keyStart = payload.indexOf(key);
  if (keyStart == -1) return -999;

  int arrayStart = payload.indexOf('[', keyStart);
  if (arrayStart == -1) return -999;

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
