#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <Arduino.h>
#include "ClientService.hpp"

typedef struct
{
  ClientService *clientService;
  void *clientServiceData; 
} CONNECTION_POOL_DATA;

class ConnectionPool {
  public:
    static CONNECTION_POOL_DATA connectionPool[];
    static uint16_t maxPoolConnections;

    int16_t addConnection(ClientService *clientService, void *clientServiceData) {
      for(uint16_t i = 0; i < maxPoolConnections; i++) {
        if(connectionPool[i].clientService == nullptr) {
          connectionPool[i].clientService = clientService;
          connectionPool[i].clientServiceData = clientServiceData;
          return i;
        }
      }
      return -1;
    }

    void removeConnection(int16_t index) {
      connectionPool[index].clientService = nullptr;
      connectionPool[index].clientServiceData = nullptr;
    }

    int16_t getConnectionIndex(ClientService *clientService, void *clientServiceData) {
      for(uint16_t i = 0; i < maxPoolConnections; i++) {
        if(connectionPool[i].clientService == clientService && connectionPool[i].clientServiceData == clientServiceData) {
          return i;
        }
      }
      return -1;
    }

    CONNECTION_POOL_DATA getPoolEntry(int16_t connectionId) {
      return connectionPool[connectionId];
    }
};

CONNECTION_POOL_DATA ConnectionPool::connectionPool[] = {
  { nullptr, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr }
};

uint16_t ConnectionPool::maxPoolConnections = (uint16_t)(sizeof(connectionPool) / sizeof(CONNECTION_POOL_DATA));

#endif