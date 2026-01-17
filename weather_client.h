#ifndef weather_client_h
#define weather_client_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class WeatherClient {
  public:
    WeatherClient();
    void update();
    int getTemperature(bool tomorrow);
    int getWeatherCode(bool tomorrow);
    float getSunshineDuration(bool tomorrow);
    bool isDataValid();
    void invalidateCache();  // Force refresh on next update

  private:
    float tempTodayNoon;
    float tempTomorrowNoon;
    int codeTodayNoon;
    int codeTomorrowNoon;
    float sunshineToday;
    float sunshineTomorrow;
    unsigned long lastUpdate;
    unsigned long lastAttempt;  // Rate-limit failed connection attempts
    bool dataValid;
    
    // Coordinates for Oedheim, Germany
    // 49.2394° N, 9.2553° E
    const char* host = "api.open-meteo.com";
    const String url = "/v1/forecast?latitude=49.2394&longitude=9.2553&hourly=temperature_2m,weathercode&daily=sunshine_duration&forecast_days=2&timezone=Europe%2FBerlin";

    void parseJson(String payload);
    float extractValueAtIndex(String payload, String key, int index);
    float extractDailyValueAtIndex(String payload, String key, int index);
};

#endif
