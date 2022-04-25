#include "BleServerService.hpp"

BleServerService::BleServerService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool)
  : _connectionPool(connectionPool)
  , csConnection(csAtConnection)
{
}

BleServerService::~BleServerService()
{
}

void BleServerService::setup() {
  // Create the BLE Device
  BLEDevice::init("CommandStation EX");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(this);

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
										);

  pRxCharacteristic->setCallbacks((BLECharacteristicCallbacks*)this);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

void BleServerService::loop() {

}

void BleServerService::send(void *clientData, const char *data) {
  Serial.print("BLE SEND VALUE");
  pTxCharacteristic->setValue((uint8_t*)data, strlen(data));
  pTxCharacteristic->notify();
}

void BleServerService::onConnect(BLEServer* pServer) {
  Serial.print("BLE CONNECT");
  deviceConnected = true;
  pTxCharacteristic->setValue((uint8_t*)"xxx", 3);
  pTxCharacteristic->notify();
  uint16_t connectionIndex = _connectionPool->addConnection(this, nullptr);
  csConnection->sendConnect(connectionIndex);
  csConnection->sendData(connectionIndex, 5, (uint8_t*)"<s>\r\n");
};

void BleServerService::onDisconnect(BLEServer* pServer) {
  Serial.print("BLE DISCONNECT");
  deviceConnected = false;
  uint16_t connectionIndex = _connectionPool->getConnectionIndex(this, nullptr);
  csConnection->sendDisconnect(connectionIndex);
  _connectionPool->removeConnection(connectionIndex);
  pServer->startAdvertising(); // restart advertising
}

void BleServerService::onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) {
  Serial.println("+++++++++++");
  std::string rxValue = pCharacteristic->getValue();
  if (rxValue.length() > 0) {
    uint16_t connectionIndex = _connectionPool->getConnectionIndex(this, nullptr);
    csConnection->sendData(connectionIndex, rxValue.length(), (uint8_t*)rxValue.c_str());
    Serial.println("*********");
    Serial.print("Received Value: ");
    for (int i = 0; i < rxValue.length(); i++)
      Serial.print(rxValue[i]);

    Serial.println();
    Serial.println("*********");
  }
}
