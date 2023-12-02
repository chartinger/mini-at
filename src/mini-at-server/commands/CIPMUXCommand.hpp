#ifndef CIPMUXCOMMAND
#define CIPMUXCOMMAND

#include "MiniAtParserCommandHandler.hpp"
#include <Arduino.h>

class CIPMUXCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+CIPMUX"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { return 0; };
};

#endif
