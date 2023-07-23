#ifndef BLE_SERVER_SERVICE_H
#define BLE_SERVER_SERVICE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ClientService.hpp"
#include "ConnectionPool.hpp"
#include "CsAtConnection.hpp"

#define SERVICE_UUID           "93d3c8e8-48c7-4b88-b3be-e1e7723bd884"
#define CHARACTERISTIC_UUID_RX "3aea00a1-37da-421c-8b7d-64be3a254f0b"
#define CHARACTERISTIC_UUID_TX "a4f5be33-d9fd-431b-be3f-cf3146082ba2"

class BleServerService : ClientService, public BLEServerCallbacks, public BLECharacteristicCallbacks {
  public:
    BleServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool);
    virtual ~BleServerService();
    virtual void setup();
    virtual void loop();
    virtual void send(void *clientData, const char *data);

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    virtual void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param);

  private:
    ConnectionPool *_connectionPool;
    CsAtConnection *csConnection;
    boolean deviceConnected = false;
    BLEServer *pServer = nullptr;
    BLECharacteristic *pTxCharacteristic = nullptr;
};

#endif
