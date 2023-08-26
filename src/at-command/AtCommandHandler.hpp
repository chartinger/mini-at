#ifndef AT_COMMAND_HANDLER_H
#define AT_COMMAND_HANDLER_H

class AtParser;

typedef int16_t AT_COMMAND_RETURN_TYPE;
typedef class AtCommandHandler AtCommandHandler;

class AtCommandHandler {
public:
  AtCommandHandler() {}
  virtual ~AtCommandHandler() {}
  virtual const char *getName() = 0;
  virtual AT_COMMAND_RETURN_TYPE run(AtParser *at_parser) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE test(AtParser *at_parser) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE read(AtParser *at_parser) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE write(AtParser *at_parser, char **argv, uint16_t argc) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE passthrough(AtParser *at_parser, char* data, uint16_t dataLength) { return -1; };
};

#endif