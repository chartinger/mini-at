#ifndef GMRCOMMAND
#define GMRCOMMAND

#include <Arduino.h>
#include "MiniAtParserCommandHandler.hpp"

class GMRCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+GMR"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    out_stream->println("AT mini custom firmware");
    return 0;
  };
};

#endif
