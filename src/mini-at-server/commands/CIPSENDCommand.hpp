#ifndef CIPSEND_COMMAND_H
#define CIPSEND_COMMAND_H

#include <Arduino.h>
#include "../pool/ConnectionPool.hpp"
#include "MiniAtParserCommandHandler.hpp"

typedef class CIPSENDCommand CIPSENDCommand;

class CIPSENDCommand : public MiniAtParserCommandHandler {
 public:
  CIPSENDCommand(ConnectionPool* connectionPool) : connectionPool(connectionPool) {}

  const char* getName() { return "+CIPSEND"; };

  AT_COMMAND_RETURN_TYPE write(Stream* out_stream, char** argv, uint16_t argc) {
    if (argc != 2) {
      return -1;
    }
    int16_t payloadConnection = atoi(argv[0]);
    int16_t payloadLength = atoi(argv[1]);
    if (payloadConnection < 0 || payloadLength <= 0) {
      return -1;
    }
    passthroughConnectionIndex = payloadConnection;
    return payloadLength;
  };

  AT_COMMAND_RETURN_TYPE passthrough(Stream* out_stream, char* data, uint16_t dataLength) {
    if (passthroughConnectionIndex >= 0) {
      auto poolEntry = (this->connectionPool)->getPoolEntry(passthroughConnectionIndex);
      if (poolEntry.clientService != nullptr) {
        poolEntry.clientService->send(poolEntry.clientServiceData, data);
        passthroughConnectionIndex = -1;
        return 0;
      }
      passthroughConnectionIndex = -1;
      return -1;
    }
    passthroughConnectionIndex = -1;
    return 0;
  }

 private:
  int16_t passthroughConnectionIndex = -1;
  ConnectionPool* connectionPool;
};

#endif
