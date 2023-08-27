#include "MqttClientService.hpp"

MqttClientService::MqttClientService(CsAtConnection *csAtConnection, ConnectionPool *connectionPool, WiFiClient *wifiClient)
  : _connectionPool(connectionPool)
  , csConnection(csAtConnection)
  , mqttClient(*wifiClient)
{
}

void MqttClientService::setup() {
  mqttClient.setServer(MQTT_BROKER_URL, MQTT_BROKER_PORT);
  mqttClient.setCallback([=](char* topic, byte* payload, unsigned int length) { handleMqttMessage(topic, payload, length); });
}

void MqttClientService::send(void *clientData, const char *data) {
  mqttClient.publish(MQTT_TOPIC_OUT, data);
}

void MqttClientService::handleMqttMessage(char* topic, byte* payload, unsigned int length) {
  uint16_t clientId = _connectionPool->getConnectionIndex(this, nullptr);
  csConnection->sendData(clientId, length, payload);
}

void MqttClientService::reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = MQTT_CLIENT_ID_PREFIX;
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), NULL, NULL, MQTT_TOPIC_STATUS, 0, true, "offline")) {
      Serial.println("connected");
      mqttClient.publish(MQTT_TOPIC_STATUS, "online", true);
      mqttClient.publish(MQTT_TOPIC_STATUS, WiFi.localIP().toString().c_str());
      mqttClient.subscribe(MQTT_TOPIC_IN);
      _connectionPool->addConnection(this, nullptr);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      // Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

MqttClientService::~MqttClientService() {
}

void MqttClientService::loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}
