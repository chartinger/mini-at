#ifndef MQTT_CLIENT_SERVICE_H
#define MQTT_CLIENT_SERVICE_H

#include "config.h"

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>

#include "ClientService.hpp"
#include "../pool/ConnectionPool.hpp"

#include "../CsAtConnection.hpp"

class MqttClientService : ClientService {
  public:
    MqttClientService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, WiFiClient *wifiClient);
    virtual ~MqttClientService();
    virtual void setup();
    virtual void loop();
    virtual void send(void *clientData, const char *data);

  private:
    ConnectionPool *_connectionPool;
    CsAtConnection *csConnection;
    PubSubClient mqttClient;

    void reconnect();
    void handleMqttMessage(char* topic, byte* payload, unsigned int length);
};

#endif
