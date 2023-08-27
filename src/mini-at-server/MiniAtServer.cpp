#include "./MiniAtServer.hpp"

void MiniAtServer::begin(Stream *stream) {
  this->stream = stream;
  this->atParser.begin(stream, commands, sizeof(commands), at_buffer, sizeof(at_buffer));
  tcpService.setup();
  wsService.setup();
}

void MiniAtServer::loop() {
  while (stream->available() > 0) {
    int ch = stream->read();
    if (echo) {
      stream->print((char)ch);
    }
    atParser.parse(ch);
  }
  wsService.loop();
  tcpService.loop();
}
