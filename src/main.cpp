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

#define WORKING_BUFFER_SIZE 255 // The size of the working buffer (ie: the expected length of the input string)

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

typedef enum
{
    CONNECTION_TYPE_NONE,
    CONNECTION_TYPE_WS,
} CONNECTION_TYPE;

typedef struct
{
    CONNECTION_TYPE connectionType;
    AsyncWebSocketClient *wsClient;
} CONNECTION_DATA;

static CONNECTION_DATA connectionData[] = {
    {CONNECTION_TYPE_NONE, nullptr},
    {CONNECTION_TYPE_NONE, nullptr},
    {CONNECTION_TYPE_NONE, nullptr},
    {CONNECTION_TYPE_NONE, nullptr}
};
static uint16_t maxConnections = (uint16_t)(sizeof(connectionData) / sizeof(CONNECTION_DATA));

int16_t addConnection(AsyncWebSocketClient *wsClient) {
  for(uint16_t i = 0; i < maxConnections; i++) {
    if(connectionData[i].connectionType == CONNECTION_TYPE_NONE) {
      connectionData[i].connectionType = CONNECTION_TYPE_WS;
      connectionData[i].wsClient = wsClient;
      return i;
    }
  }
  return -1;
}

AsyncWebSocketClient *getWebsocketClient(int16_t index) {
  return connectionData[index].wsClient;
}


int16_t getConnectionIndex(AsyncWebSocketClient const *wsClient) {
  for(uint16_t i = 0; i < maxConnections; i++) {
    if(connectionData[i].wsClient == wsClient) {
      return i;
    }
  }
  return -1;
}

void removeConnection(int16_t index) {
    connectionData[index].connectionType = CONNECTION_TYPE_NONE;
    connectionData[index].wsClient = nullptr;
}

void removeConnection(AsyncWebSocketClient const *wsClient) {
  for(uint16_t i = 0; i < maxConnections; i++) {
    if(connectionData[i].wsClient == wsClient) {
      removeConnection(i);
      return;
    }
  }
}

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

#ifdef WEBSOCKET_ENABLED
AsyncWebServer webserver(80);
AsyncWebSocket ws("/ws");

int16_t passthroughConnectionIndex = -1;

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint16_t clientId) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    AT.serial->print(F("IPD,"));
    AT.serial->print(clientId);
    AT.serial->print(F(","));
    AT.serial->print(len);
    AT.serial->print(F(":"));
    AT.serial->println((char*)data);
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  int16_t connectionIndex = -1;
  switch (type) {
    case WS_EVT_CONNECT:
      connectionIndex = addConnection(client);
      if(connectionIndex < 0) {
        client->close();
        break;
      }
      AT.serial->print(connectionIndex);
      AT.serial->println(F(",CONNECTED"));
      break;
    case WS_EVT_DISCONNECT:
      connectionIndex = getConnectionIndex(client);
      if(connectionIndex > 0) {
        AT.serial->print(connectionIndex);
        AT.serial->println(F(",CLOSED"));
        removeConnection(connectionIndex);
      }
      break;
    case WS_EVT_DATA:
      connectionIndex = getConnectionIndex(client);
      handleWebSocketMessage(arg, data, len, connectionIndex);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


void setupWebsocket() {
  ws.onEvent(onWebSocketEvent);
  webserver.addHandler(&ws);
  webserver.begin();
}
#endif

AT_COMMAND_RETURN_TYPE ping(ATCommands *sender)
{
    // sender->next() is NULL terminated ('\0') if there are no more parameters
    // so check for that or a length of 0.
    Serial.println(F("pong"));
    return 2; // tells ATCommands to print OK
}

AT_COMMAND_RETURN_TYPE printEspInfo(ATCommands *sender)
{
    sender->serial->println(ESP.getFreeHeap());
    return 0;
}

AT_COMMAND_RETURN_TYPE passthrough(ATCommands *sender)
{
    if(passthroughConnectionIndex >= 0) {
      auto client = getWebsocketClient(passthroughConnectionIndex);
      if(client == nullptr) {
        return -1;
      }
      client->text(sender->getBuffer());
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
    sender->serial->print(F("+CIFSR:STAIP,"));
    sender->serial->println(WiFi.localIP());
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
    auto connection = getWebsocketClient(payloadConnection);
    if(connection == nullptr) {
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
    // put your setup code here, to run once:
    Serial.begin(115200);
    setupWifi();
    AT.begin(&Serial, commands, sizeof(commands), WORKING_BUFFER_SIZE);
#ifdef WEBSOCKET_ENABLED
    setupWebsocket();
#endif
    Serial.println("STARTING");
}

void loop()
{
    // put your main code here, to run repeatedly:
    AT.update();
}