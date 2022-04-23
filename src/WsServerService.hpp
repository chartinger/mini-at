#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#ifdef ESP32
#include <AsyncTCP.h>
#endif
#ifdef ESP8266
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <ATCommands.h>
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

WsServerService::WsServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, uint16_t port)
  : webserver(port)
  , _connectionPool(connectionPool)
  , ws("/ws")
  , csConnection(csAtConnection)
{
}

WsServerService::~WsServerService() {
  this->webserver.end();
}

void WsServerService::setup() {
  ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) { onWebSocketEvent(server, client, type, arg, data, len); });
  webserver.addHandler(&ws);
  webserver.begin();
}

void WsServerService::loop() {
  this->ws.cleanupClients();
}

void WsServerService::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  int16_t connectionIndex = -1;
  switch (type) {
    case WS_EVT_CONNECT:
      connectionIndex = _connectionPool->addConnection(this, client);
      if(connectionIndex < 0) {
        client->close();
        break;
      }
      csConnection->sendConnect(connectionIndex);
      break;
    case WS_EVT_DISCONNECT:
      connectionIndex = _connectionPool->getConnectionIndex(this, client);
      if(connectionIndex >= 0) {
        csConnection->sendDisconnect(connectionIndex);
        _connectionPool->removeConnection(connectionIndex);
      }
      break;
    case WS_EVT_DATA:
      connectionIndex = _connectionPool->getConnectionIndex(this, client);
      handleWebSocketMessage(arg, data, len, connectionIndex);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void WsServerService::handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint16_t clientId) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    csConnection->sendData(clientId, len, data);
  }
}

void WsServerService::send(void *clientData, const char *data) {
  ((AsyncWebSocketClient*)clientData)->text(data);
}


#endif