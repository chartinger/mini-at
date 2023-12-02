#ifndef CIFSRCOMMAND
#define CIFSRCOMMAND

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include "MiniAtParserCommandHandler.hpp"

class MiniAtServer;

class CIFSRCommand : public MiniAtParserCommandHandler {
public:
  CIFSRCommand() {}

  const char *getName() { return "+CIFSR"; };

  AT_COMMAND_RETURN_TYPE run(Stream *out_stream);
};

#endif
