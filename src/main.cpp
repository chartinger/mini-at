#include "./config.h"
#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
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

#include <ATCommands.h>

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


#define WORKING_BUFFER_SIZE 255 // The size of the working buffer (ie: the expected length of the input string)

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

#define RX 16
#define TX 17

ATCommands AT; // create an instance of the class
String str1;
String str2;
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

#ifdef WEBSOCKET_ENABLED
WsServerService wsService(&csAtConnection, &connectionPool, 80);
#endif
TcpServerService tcpService(&csAtConnection, &connectionPool, 9999);


AT_COMMAND_RETURN_TYPE ping(ATCommands *sender)
{
    Serial.println(F("pong"));
    return 0;
}

AT_COMMAND_RETURN_TYPE printEspInfo(ATCommands *sender)
{
    sender->serial->println(ESP.getFreeHeap());
    return 0;
}

AT_COMMAND_RETURN_TYPE passthrough(ATCommands *sender)
{
    if(passthroughConnectionIndex >= 0) {
      auto poolEntry = connectionPool.getPoolEntry(passthroughConnectionIndex);
      if(poolEntry.clientService != nullptr) {
        poolEntry.clientService->send(poolEntry.clientServiceData, sender->getBuffer().c_str());
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

AT_COMMAND_RETURN_TYPE printVersion(ATCommands *sender)
{
    sender->serial->println(F("AT mini custom firmware"));
    return 0;
}

AT_COMMAND_RETURN_TYPE printWifiInfo(ATCommands *sender)
{
    sender->serial->print(F("+CIFSR:STAIP,\""));
    sender->serial->print(WiFi.localIP());
    sender->serial->print(F("\""));
    sender->serial->print(F("+CIFSR:STAMAC,"));
    sender->serial->println(WiFi.macAddress());
    return 0;
}

AT_COMMAND_RETURN_TYPE ok(ATCommands *sender)
{
    return 0;
}

AT_COMMAND_RETURN_TYPE enableEchoMode(ATCommands *sender)
{
    sender->setEchoMode(true);
    return 0;
}

AT_COMMAND_RETURN_TYPE disableEchoMode(ATCommands *sender)
{
    sender->setEchoMode(false);
    return 0;
}

AT_COMMAND_RETURN_TYPE startSend(ATCommands *sender)
{
    str1 = sender->next();
    if (str1.length() == 0) {
      return -1;
    }
    str2 = sender->next();
    if (str2.length() == 0) {
      return -1;
    }
    payloadConnection = str1.toInt();
    int16_t payloadLength = str2.toInt();
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
    Serial2.begin(115200, SERIAL_8N1, RX, TX);
    setupWifi();
    AT.begin(&Serial2, commands, sizeof(commands), WORKING_BUFFER_SIZE);
#ifdef WEBSOCKET_ENABLED
    wsService.setup();
#endif
    tcpService.setup();
    Serial.println("STARTING");
}

void loop()
{
  AT.update();
  #ifdef WEBSOCKET_ENABLED
  wsService.loop();
  #endif
  tcpService.loop();
  #ifdef OTA_ENABLED
  ArduinoOTA.handle();
  #endif
}
