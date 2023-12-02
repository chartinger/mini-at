#ifndef CIPSERVERCOMMAND
#define CIPSERVERCOMMAND

#include "MiniAtParserCommandHandler.hpp"
#include <Arduino.h>

class CIPSERVERCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIPSERVER"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { return 0; };
};

#endif
