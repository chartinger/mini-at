#ifndef MDNSCOMMAND
#define MDNSCOMMAND

#include "../../mini-at-parser/MiniAtParserCommandHandler.hpp"
#include <Arduino.h>

class MDNSCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+MDNS"; };
  AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { return 0; };
};

#endif
