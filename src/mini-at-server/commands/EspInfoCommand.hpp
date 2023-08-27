#ifndef ESP_INFO_COMMAND
#define ESP_INFO_COMMAND

#include <Arduino.h>
#include "../../mini-at-parser/MiniAtParserCommandHandler.hpp"

class EspInfoCommand : public MiniAtParserCommandHandler {
  const char *getName() { return "+ESPINFO"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { 
    out_stream->println(ESP.getFreeHeap());
    return 0;
  };
};

#endif
