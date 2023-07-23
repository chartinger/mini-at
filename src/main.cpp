#include "./config.h"
#include <Arduino.h>

#include "at-command/AtParser.hpp"

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#define CsSerial Serial
#endif
#ifdef ESP8266
#define CsSerial Serial1
#endif

#ifdef WEBSOCKET_ENABLED
  #ifdef ESP32
  #include <AsyncTCP.h>
  #endif
  #ifdef ESP8266
  #include <ESPAsyncTCP.h>
  #endif
  #include <ESPAsyncWebServer.h>
#endif

// #include <AtParser.h>

#include "CsAtConnection.hpp"
#include "ConnectionPool.hpp"

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

#ifdef WEBSOCKET_ENABLED
  #include "WsServerService.hpp"
#endif

#include "TcpServerService.hpp"

#ifdef MQTT_ENABLED
#include "MqttClientService.hpp"
#endif

#define WORKING_BUFFER_SIZE 255 // The size of the working buffer (ie: the expected length of the input string)
char at_buffer[WORKING_BUFFER_SIZE];

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

#define RX 16
#define TX 17

WiFiClient wifiClient;

AtParser AT; // create an instance of the class
const char* str1;
const char* str2;
int16_t payloadConnection = -1;

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

CsAtConnection csAtConnection(&AT);
ConnectionPool connectionPool;
int16_t passthroughConnectionIndex = -1;

#ifdef BLE_ENABLED
#include "BleServerService.hpp"
BleServerService bleService(&csAtConnection, &connectionPool);
#endif

#ifdef WEBSOCKET_ENABLED
WsServerService wsService(&csAtConnection, &connectionPool, 80);
#endif
TcpServerService tcpService(&csAtConnection, &connectionPool, 9999);

#ifdef MQTT_ENABLED
MqttClientService mqttService(&csAtConnection, &connectionPool, &wifiClient);
#endif


AT_COMMAND_RETURN_TYPE ping(AtParser *sender)
{
    Serial.println(F("pong"));
    return 0;
}

AT_COMMAND_RETURN_TYPE printEspInfo(AtParser *sender)
{
    sender->serial->println(ESP.getFreeHeap());
    return 0;
}

AT_COMMAND_RETURN_TYPE passthrough(AtParser *sender)
{
    if(passthroughConnectionIndex >= 0) {
      auto poolEntry = connectionPool.getPoolEntry(passthroughConnectionIndex);
      if(poolEntry.clientService != nullptr) {
        poolEntry.clientService->send(poolEntry.clientServiceData, sender->getBuffer());
        passthroughConnectionIndex = -1;
        return 0;
      }
      Serial.print(F("NO PASSTHROUGH HANDLER: "));
      Serial.println(passthroughConnectionIndex);
      passthroughConnectionIndex = -1;
      return -1;
    }
    passthroughConnectionIndex = -1;
    return 0;
}

AT_COMMAND_RETURN_TYPE printVersion(AtParser *sender)
{
    sender->serial->println(F("AT mini custom firmware"));
    return 0;
}

AT_COMMAND_RETURN_TYPE printWifiInfo(AtParser *sender)
{
    sender->serial->print(F("+CIFSR:STAIP,\""));
    sender->serial->print(WiFi.localIP());
    sender->serial->print(F("\""));
    sender->serial->print(F("+CIFSR:STAMAC,"));
    sender->serial->println(WiFi.macAddress());
    return 0;
}

AT_COMMAND_RETURN_TYPE ok(AtParser *sender)
{
    return 0;
}

AT_COMMAND_RETURN_TYPE enableEchoMode(AtParser *sender)
{
    // sender->setEchoMode(true);
    return 0;
}

AT_COMMAND_RETURN_TYPE disableEchoMode(AtParser *sender)
{
    // sender->setEchoMode(false);
    return 0;
}

AT_COMMAND_RETURN_TYPE startSend(AtParser *sender)
{
    str1 = sender->getNextParameter();
    if (str1 == nullptr) {
      return -1;
    }
    str2 = sender->getNextParameter();
    if (str2 == nullptr) {
      return -1;
    }
    payloadConnection = atoi(str1);
    int16_t payloadLength = atoi(str2);
    if(payloadConnection < 0 || payloadLength <=0) {
      return -1;
    }
    passthroughConnectionIndex = payloadConnection;
    return payloadLength;
}

static at_command_t commands[] = {
    {"+GMR", printVersion, nullptr, nullptr, nullptr, nullptr},
    {"+CIFSR", printWifiInfo, nullptr, nullptr, nullptr, nullptr},
    // {"+CIPMUX", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CIPRECVMODE", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CIPSERVER", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CWHOSTNAME", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CWJAP", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CWJAP_CUR", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CWMODE", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+CWSAP", ping, nullptr, nullptr, nullptr, nullptr},
    // {"+MDNS", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIPSEND", nullptr, nullptr, nullptr, startSend, passthrough},
    {"+RST", ping, nullptr, nullptr, nullptr, passthrough},
    {"E0", disableEchoMode, nullptr, nullptr, nullptr, nullptr},
    {"E1", enableEchoMode, nullptr, nullptr, nullptr, nullptr},
    {"+ESPINFO", printEspInfo, nullptr, nullptr, nullptr, nullptr},
    {"", ok, nullptr, nullptr, nullptr, nullptr}
};

void setup()
{
    Serial.begin(115200);
    #ifdef ESP32
      // CsSerial.begin(115200, SERIAL_8N1, RX, TX);
      CsSerial.begin(115200);
    #endif
    #ifdef ESP8266
      CsSerial.begin(115200);
    #endif
    setupWifi();
    AT.begin(&CsSerial, commands, sizeof(commands), at_buffer);
#ifdef WEBSOCKET_ENABLED
    wsService.setup();
#endif
    tcpService.setup();
#ifdef MQTT_ENABLED
    mqttService.setup();
#endif
#ifdef BLE_ENABLED
    bleService.setup();
#endif
    Serial.println("STARTING");
}

void loop()
{ 
    while (CsSerial.available() > 0)
    {
        int ch = CsSerial.read();
        CsSerial.print((char)ch);
        AT.parse(ch);
    }

//  AT.update();
  #ifdef WEBSOCKET_ENABLED
  // wsService.loop();
  #endif
  // tcpService.loop();
  #ifdef OTA_ENABLED
  ArduinoOTA.handle();
  #endif
  #ifdef MQTT_ENABLED
  mqttService.loop();
  #endif
  #ifdef BLE_ENABLED
  bleService.loop();
  #endif
}
