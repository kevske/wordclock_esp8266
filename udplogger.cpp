#include "udplogger.h"
#include <ESP8266WiFi.h>

UDPLogger::UDPLogger(){

}

UDPLogger::UDPLogger(IPAddress interfaceAddr, IPAddress multicastAddr, int port){
    _multicastAddr = multicastAddr;
    _port = port;
    _interfaceAddr = interfaceAddr;
    _name = "Log";
    _Udp.beginMulticast(_interfaceAddr, _multicastAddr, _port);
}

void UDPLogger::setName(String name){
    _name = name;
}

void UDPLogger::logString(String logmessage){
    // wait 5 milliseconds if last send was less than 5 milliseconds before 
    if(millis() < (_lastSend + 5)){
        delay(5);
    }
    logmessage = _name + ": " + logmessage;
    Serial.println(logmessage);

    // If WiFi is not connected, skip UDP multicast to avoid blocking/failures
    if (WiFi.status() != WL_CONNECTED) {
        _lastSend = millis();
        return;
    }

    // If interface address is 0.0.0.0, skip
    if (_interfaceAddr == IPAddress(0,0,0,0)) {
        _lastSend = millis();
        return;
    }
    _Udp.beginPacketMulticast(_multicastAddr, _port, _interfaceAddr);
    // Truncate message if too long, using the defined buffer size
    logmessage.toCharArray(_packetBuffer, UDP_LOG_BUFFER_SIZE);
    _Udp.print(_packetBuffer);
    _Udp.endPacket();
    _lastSend=millis();
}

void UDPLogger::logColor24bit(uint32_t color){
  uint8_t resultRed = color >> 16 & 0xff;
  uint8_t resultGreen = color >> 8 & 0xff;
  uint8_t resultBlue = color & 0xff;
  logString(String(resultRed) + ", " + String(resultGreen) + ", " + String(resultBlue));
}

void UDPLogger::refreshInterface(IPAddress interfaceAddr){
    _interfaceAddr = interfaceAddr;
    // Restart UDP and rejoin multicast group on the new interface
    _Udp.stop();
    if (WiFi.status() == WL_CONNECTED) {
        _Udp.beginMulticast(_interfaceAddr, _multicastAddr, _port);
    }
}

void UDPLogger::logPrintf(const char *format, ...) {
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return;
    };
    if (len >= sizeof(loc_buf)) {
        temp = (char*) malloc(len+1);
        if (temp == NULL) {
            va_end(arg);
            return;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    
    logString(String(temp));
    
    if (temp != loc_buf) {
        free(temp);
    }
}