#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <AsyncTCP.h>
#endif
#ifdef ESP8266
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include "CsAtConnection.hpp"
#include "ClientService.hpp"
#include "ConnectionPool.hpp"

class WsServerService : ClientService {
  public:
    WsServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, uint16_t port);
    virtual ~WsServerService();
    virtual void setup();
    virtual void loop();
    virtual void send(void *clientData, const char *data);

  private:
    AsyncWebServer webserver;
    ConnectionPool *_connectionPool;
    AsyncWebSocket ws;
    CsAtConnection *csConnection;
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint16_t clientId);
};

#endif
