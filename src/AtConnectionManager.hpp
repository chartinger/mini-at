#ifndef AT_CONNECTION_MANAGER_H
#define AT_CONNECTION_MANAGER_H

#include <ATCommands.h>

class AtConnectionManager {
  public:
    AtConnectionManager(ATCommands *AT);
    void sendData(uint16_t connectionId, size_t len, uint8_t *data);
    void sendDisconnect(uint16_t connectionId);
    void sendConnect(uint16_t connectionId);

  private:
    ATCommands *AT;
};

AtConnectionManager::AtConnectionManager(ATCommands *AtCommands)
  : AT(AtCommands) {}

void AtConnectionManager::sendData(uint16_t connectionId, size_t len, uint8_t *data) {
    AT->serial->print(F("+IPD,"));
    AT->serial->print(connectionId);
    AT->serial->print(F(","));
    AT->serial->print(len);
    AT->serial->print(F(":"));
    AT->serial->write(data, len);
    AT->serial->println();
}

void AtConnectionManager::sendDisconnect(uint16_t connectionId) {
  AT->serial->print(connectionId);
  AT->serial->println(F(",CLOSED"));
}

void AtConnectionManager::sendConnect(uint16_t connectionId) {
  AT->serial->print(connectionId);
  AT->serial->println(F(",CONNECTED"));
}

#endif
