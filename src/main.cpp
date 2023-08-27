#include "./config.h"
#include <Arduino.h>

#include "mini-at-server/MiniAtServer.hpp"

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#define CsSerial Serial2
#endif
#ifdef ESP8266
#define CsSerial Serial1
#endif

#ifdef OTA_ENABLED
  #include <ArduinoOTA.h>
#endif

#ifdef MDNS_HOSTNAME
  #ifdef ESP32
  #include <ESPmDNS.h>
  #endif
  #ifdef ESP8266
  #include <ESP8266mDNS.h> 
  #endif
#endif

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

#define RX 16
#define TX 17


void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
#ifdef MDNS_HOSTNAME
  MDNS.begin(MDNS_HOSTNAME);
#endif

#ifdef MDNS_HOSTNAME
#ifdef OTA_ENABLED
ArduinoOTA.setHostname(MDNS_HOSTNAME);
#endif
#endif

#ifdef OTA_ENABLED
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
#endif
  ArduinoOTA.begin();
#endif
}

MiniAtServer miniAtServer;

void setup()
{
    Serial.begin(115200);
    #ifdef ESP32
      CsSerial.begin(115200, SERIAL_8N1, RX, TX);
    #endif
    #ifdef ESP8266
      CsSerial.begin(115200);
    #endif
    setupWifi();
    miniAtServer.begin(&CsSerial);
    Serial.println("STARTING");
}

void loop()
{ 
  miniAtServer.loop();
  #ifdef OTA_ENABLED
  ArduinoOTA.handle();
  #endif
}
