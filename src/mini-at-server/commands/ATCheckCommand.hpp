#ifndef ATCHECK_COMMAND
#define ATCHECK_COMMAND

#include <Arduino.h>
#include "../../mini-at-parser/MiniAtParserCommandHandler.hpp"

class ATCheckCommand : public MiniAtParserCommandHandler {
  const char *getName() { return ""; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    return 0;
  };
};

#endif
