#ifndef AT_COMMAND_HANDLER_H
#define AT_COMMAND_HANDLER_H

#include <Arduino.h>

typedef class AtCommandHandler AtCommandHandler;
typedef int16_t AT_COMMAND_RETURN_TYPE;
class AtParser;

class AtCommandHandler {
  public:
    AtCommandHandler() {}
    virtual ~AtCommandHandler() {}
    virtual const char* getName() { return "X"; };
    virtual AT_COMMAND_RETURN_TYPE run(AtParser *at_parser) { return -1; };
    virtual AT_COMMAND_RETURN_TYPE test(AtParser *at_parser) { return -1; };
    virtual AT_COMMAND_RETURN_TYPE read(AtParser *at_parser) { return -1; };
    virtual AT_COMMAND_RETURN_TYPE write(AtParser *at_parser) { return -1; };
    virtual AT_COMMAND_RETURN_TYPE passthrough(AtParser *at_parser) { return -1; };
};

#endif