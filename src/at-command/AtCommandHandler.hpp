#ifndef AT_COMMAND_HANDLER_H
#define AT_COMMAND_HANDLER_H

#include <Arduino.h>

typedef int16_t AT_COMMAND_RETURN_TYPE;
typedef class AtCommandHandler AtCommandHandler;

class AtCommandHandler {
public:
  AtCommandHandler() {}
  virtual ~AtCommandHandler() {}
  virtual const char *getName() = 0;
  virtual AT_COMMAND_RETURN_TYPE run(Stream *out_stream) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE test(Stream *out_stream) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE read(Stream *out_stream) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE write(Stream *out_stream, char **argv, uint16_t argc) { return -1; };
  virtual AT_COMMAND_RETURN_TYPE passthrough(Stream *out_stream, char* data, uint16_t dataLength) { return -1; };
};

#endif