#ifndef CS_AT_CONNECTION_H
#define CS_AT_CONNECTION_H

#include "./at-command/AtParser.hpp"

class CsAtConnection {
  public:
    CsAtConnection(AtParser *AT);
    void sendData(uint16_t connectionId, size_t len, const uint8_t *data);
    void sendDisconnect(uint16_t connectionId);
    void sendConnect(uint16_t connectionId);

  private:
    AtParser *AT;
};

#endif
