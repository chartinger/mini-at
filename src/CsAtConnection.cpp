#include "CsAtConnection.hpp"

CsAtConnection::CsAtConnection(ATCommands *AtCommands)
  : AT(AtCommands) {}

void CsAtConnection::sendData(uint16_t connectionId, size_t len, uint8_t *data) {
    AT->serial->print(F("+IPD,"));
    AT->serial->print(connectionId);
    AT->serial->print(F(","));
    AT->serial->print(len);
    AT->serial->print(F(":"));
    AT->serial->write(data, len);
    AT->serial->println();
}

void CsAtConnection::sendDisconnect(uint16_t connectionId) {
  AT->serial->print(connectionId);
  AT->serial->println(F(",CLOSED"));
}

void CsAtConnection::sendConnect(uint16_t connectionId) {
  AT->serial->print(connectionId);
  AT->serial->println(F(",CONNECTED"));
}
