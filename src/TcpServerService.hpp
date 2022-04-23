#ifndef TCP_SERVER_SERVICE_H
#define TCP_SERVER_SERVICE_H

#ifdef ESP32
#include <AsyncTCP.h>
#endif
#ifdef ESP8266
#include <ESPAsyncTCP.h>
#endif

#include "ClientService.hpp"
#include "ConnectionPool.hpp"
#include "CsAtConnection.hpp"

class TcpServerService : ClientService {
  public:
    TcpServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, uint16_t port);
    virtual ~TcpServerService();
    virtual void setup();
    virtual void loop();
    virtual void send(void *clientData, const char *data);

  private:
    AsyncServer tcpServer;
    ConnectionPool *_connectionPool;
    CsAtConnection *csConnection;
};

TcpServerService::TcpServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, uint16_t port)
  : tcpServer(port)
  , _connectionPool(connectionPool)
  , csConnection(csAtConnection)
{
}

void TcpServerService::setup() {
  tcpServer.onClient([=](void *s, AsyncClient* c) {
    if(c == NULL)
      return;
      uint16_t connectionIndex = _connectionPool->addConnection(this, c);
      csConnection->sendConnect(connectionIndex);
    c->onDisconnect([=](void *r, AsyncClient* c) {
      uint16_t connectionIndex = _connectionPool->getConnectionIndex(this, c);
      csConnection->sendDisconnect(connectionIndex);
      _connectionPool->removeConnection(connectionIndex);
      delete c;
    }, nullptr);
    c->onData([=](void *r, AsyncClient* c, void *buf, size_t len) {
      uint16_t clientId = _connectionPool->getConnectionIndex(this, c);
      Serial.print("TCP IN:");
      Serial.println(len);
      Serial.println((char*)buf);
      csConnection->sendData(clientId, len, (uint8_t*)buf);
    }, nullptr);
  }, nullptr);
  tcpServer.begin();
}

void TcpServerService::send(void *clientData, const char *data) {
  ((AsyncClient*)clientData)->write(data);
}

TcpServerService::~TcpServerService() {
  this->tcpServer.end();
}

void TcpServerService::loop() {
}

#endif
