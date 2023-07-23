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

    int16_t addConnection(ClientService *clientService, void *clientServiceData);
    int16_t getConnectionIndex(ClientService *clientService, void *clientServiceData);
    void removeConnection(int16_t index);

    CONNECTION_POOL_DATA getPoolEntry(int16_t connectionId);
};

#endif