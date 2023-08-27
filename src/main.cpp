#include "./config.h"
#include <Arduino.h>

#include "mini-at-parser/MiniAtParser.hpp"

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

#ifdef WEBSOCKET_ENABLED
  #ifdef ESP32
  #include <AsyncTCP.h>
  #endif
  #ifdef ESP8266
  #include <ESPAsyncTCP.h>
  #endif
  #include <ESPAsyncWebServer.h>
#endif

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

MiniAtParser AT; // create an instance of the class
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

class EspInfoCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+ESPINFO"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    out_stream->println(ESP.getFreeHeap());
    return 0;
  };
};
class GMRCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+GMR"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    Serial.println("#GMR#");
    out_stream->println("AT mini custom firmware");
    return 0;
  };
};

class CIFSRCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIFSR"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    Serial.println("#AT CIFSR#");
    out_stream->print(F("+CIFSR:STAIP,\""));
    out_stream->print(WiFi.localIP());
    out_stream->print(F("\""));
    out_stream->print(F("+CIFSR:STAMAC,"));
    out_stream->println(WiFi.macAddress());
    return 0;
  };
};

class ATCheckCommand : public MiniAtParserCommandHandler {
  const char *getName() { return ""; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    Serial.println("#AT REQUEST#");
    return 0;
  };
};

class CIPSENDCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIPSEND"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) {
    if(argc != 2) {
      return -1;
    }
    payloadConnection = atoi(argv[0]);
    int16_t payloadLength = atoi(argv[1]);
    if(payloadConnection < 0 || payloadLength <=0) {
      return -1;
    }
    passthroughConnectionIndex = payloadConnection;
    return payloadLength;
  };
  AT_COMMAND_RETURN_TYPE passthrough(Stream *out_stream, char* data, uint16_t dataLength) {
    if(passthroughConnectionIndex >= 0) {
      auto poolEntry = connectionPool.getPoolEntry(passthroughConnectionIndex);
      if(poolEntry.clientService != nullptr) {
        poolEntry.clientService->send(poolEntry.clientServiceData, data);
        passthroughConnectionIndex = -1;
        return 0;
      }
      passthroughConnectionIndex = -1;
      return -1;
    }
    passthroughConnectionIndex = -1;
    return 0;
  }
};

class CIPSERVERCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIPSERVER"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { 
    Serial.println("#CIPSERVER#");
    return 0;
  };
};

class CIPMUXCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIPMUX"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { 
    Serial.println("#+CIPMUX#");
    return 0;
  };
};

class MDNSCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+MDNS"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { 
    Serial.println("#+MDNS#");
    return 0;
  };
};

class EchoOnCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "E1"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    Serial.println("#+ATE1#");
    return 0;
  };
};

class EchoOffCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "E0"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    Serial.println("#+ATE0#");
    return 0;
  };
};

EspInfoCommand espInfo;
GMRCommand gmr;
CIFSRCommand cifsr;
ATCheckCommand atCheck;
CIPSENDCommand cipsend;
CIPSERVERCommand cipserver;
CIPMUXCommand cipmux;
EchoOnCommand echon;
EchoOffCommand echooff;
MDNSCommand mdns;

static MiniAtParserCommandHandler *commands[] = {&espInfo, &gmr, &cifsr, &atCheck, &cipsend, &cipserver, &cipmux, &echon, &echooff, &mdns};

void setup()
{
    Serial.begin(115200);
    #ifdef ESP32
      CsSerial.begin(115200, SERIAL_8N1, RX, TX);
      // CsSerial.begin(115200);
    #endif
    #ifdef ESP8266
      CsSerial.begin(115200);
    #endif
    setupWifi();
    AT.begin(&CsSerial, commands, sizeof(commands), at_buffer, sizeof(at_buffer));
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
        Serial.print((char)ch);
        AT.parse(ch);
    }

//  AT.update();
  #ifdef WEBSOCKET_ENABLED
  wsService.loop();
  #endif
  tcpService.loop();
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
