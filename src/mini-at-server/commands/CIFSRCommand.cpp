#include "CIFSRCommand.hpp"

AT_COMMAND_RETURN_TYPE CIFSRCommand::run(Stream *out_stream) {
  out_stream->print(F("+CIFSR:STAIP,\""));
  out_stream->print(WiFi.localIP());
  out_stream->print(F("\""));
  out_stream->print(F("+CIFSR:STAMAC,"));
  out_stream->println(WiFi.macAddress());
  return 0;
};