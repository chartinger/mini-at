#ifndef ECHO_COMMANDS
#define ECHO_COMMANDS

#include "MiniAtParserCommandHandler.hpp"
#include <Arduino.h>

class EchoOnCommand : public MiniAtParserCommandHandler {
public:
  EchoOnCommand(bool *echo) : echo(echo) {}
  const char *getName() { return "E1"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) {
    *echo = true;
    return 0;
  };

private:
  bool *echo;
};

class EchoOffCommand : public MiniAtParserCommandHandler {
public:
  EchoOffCommand(bool *echo) : echo(echo) {}
  const char *getName() { return "E0"; };
  AT_COMMAND_RETURN_TYPE run(Stream *out_stream) {
    *echo = false;
    return 0;
  };
  private:
  bool *echo;
};

#endif
