#ifndef CS_AT_CONNECTION_H
#define CS_AT_CONNECTION_H

#include <ATCommands.h>

class CsAtConnection {
  public:
    CsAtConnection(ATCommands *AT);
    void sendData(uint16_t connectionId, size_t len, uint8_t *data);
    void sendDisconnect(uint16_t connectionId);
    void sendConnect(uint16_t connectionId);

  private:
    ATCommands *AT;
};

#endif
