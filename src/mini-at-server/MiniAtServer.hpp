#ifndef MINI_AT_SERVER
#define MINI_AT_SERVER

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include "../mini-at-parser/MiniAtParser.hpp"

#include "./commands/ATCheckCommand.hpp"
#include "./commands/CIFSRCommand.hpp"
#include "./commands/CIPMUXCommand.hpp"
#include "./commands/CIPSENDCommand.hpp"
#include "./commands/CIPSERVERCommand.hpp"
#include "./commands/EchoCommands.hpp"
#include "./commands/EspInfoCommand.hpp"
#include "./commands/GMRCommand.hpp"
#include "./commands/MDNSCommand.hpp"
#include "./pool/ConnectionPool.hpp"

#include "./services/WsServerService.hpp"
#include "./services/TcpServerService.hpp"
#include "CsAtConnection.hpp"

typedef class MiniAtServer MiniAtServer;

class MiniAtServer {
public:
  MiniAtServer() {}
  MiniAtServer(const MiniAtServer &) = delete;
  MiniAtServer &operator=(const MiniAtServer &) = delete;

  void begin(Stream *stream);
  void loop();

private:
  char at_buffer[256];
  bool echo = false;
  Stream *stream;
  ConnectionPool connectionPool;
  MiniAtParser atParser;
  CsAtConnection csAtConnection = CsAtConnection(&atParser);
  WsServerService wsService = WsServerService(&csAtConnection, &connectionPool, 80);
  TcpServerService tcpService = TcpServerService(&csAtConnection, &connectionPool, 9999);

  CIPSENDCommand cipsend = CIPSENDCommand(&connectionPool);
  ATCheckCommand atCheck;
  GMRCommand gmr;
  CIFSRCommand cifsr;
  CIPSERVERCommand cipserver;
  CIPMUXCommand cipmux;
  EchoOnCommand echon = EchoOnCommand(&echo);
  EchoOffCommand echooff = EchoOffCommand(&echo);
  MDNSCommand mdns;
  EspInfoCommand espInfo;

  MiniAtParserCommandHandler *commands[10] = {
      &cipsend, &atCheck, &cifsr, &gmr, &cipserver, &cipmux, &echon, &echooff, &mdns, &espInfo,
  };
};

#endif
