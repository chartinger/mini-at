#ifndef TCP_SERVER_SERVICE_H
#define TCP_SERVER_SERVICE_H

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

#include "../pool/ConnectionPool.hpp"
#include "ClientService.hpp"

#include "../CsAtConnection.hpp"

class TcpServerService : ClientService {
 public:
  TcpServerService(CsAtConnection* csAtConnection, ConnectionPool* connectionPool, uint16_t port);
  virtual ~TcpServerService();
  virtual void setup();
  virtual void loop();
  virtual void send(void* clientData, const char* data);

 private:
  AsyncServer tcpServer;
  ConnectionPool* _connectionPool;
  CsAtConnection* csConnection;
};

#endif
