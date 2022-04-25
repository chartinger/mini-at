#include "ConnectionPool.hpp"

CONNECTION_POOL_DATA ConnectionPool::connectionPool[] = {
  { nullptr, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr }
};

uint16_t ConnectionPool::maxPoolConnections = (uint16_t)(sizeof(connectionPool) / sizeof(CONNECTION_POOL_DATA));

int16_t ConnectionPool::addConnection(ClientService *clientService, void *clientServiceData) {
  for(uint16_t i = 0; i < maxPoolConnections; i++) {
    if(connectionPool[i].clientService == nullptr) {
      connectionPool[i].clientService = clientService;
      connectionPool[i].clientServiceData = clientServiceData;
      return i;
    }
  }
  return -1;
}

void ConnectionPool::removeConnection(int16_t index) {
  if((index < 0) || (index > maxPoolConnections - 1)) return;
  connectionPool[index].clientService = nullptr;
  connectionPool[index].clientServiceData = nullptr;
}

int16_t ConnectionPool::getConnectionIndex(ClientService *clientService, void *clientServiceData) {
  for(uint16_t i = 0; i < maxPoolConnections; i++) {
    if(connectionPool[i].clientService == clientService && connectionPool[i].clientServiceData == clientServiceData) {
      return i;
    }
  }
  return -1;
}

CONNECTION_POOL_DATA ConnectionPool::getPoolEntry(int16_t connectionId) {
  return connectionPool[connectionId];
}
