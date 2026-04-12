#include "MqttClient.h"
#include <Arduino.h>


IPAddress _local_IP(192, 168, 1, 1);
IPAddress _gateway(192, 168, 1, 1);
IPAddress _subnet(255, 255, 255, 0);
IPAddress _mqttBrocker(192, 168, 1, 2);

    WiFiClient espClient;
    PubSubClient client(espClient);

MqttClient::MqttClient()
    : _enabled(false), _lastSendTime(0) {
}

MqttClient& MqttClient::getInstance() {
    static MqttClient instance;
    return instance;
}

bool MqttClient::enable() {
  if(startWifi()){
    connectMqtt();
    _enabled = true;
    return true;
  }
  return false;
}

void MqttClient::disable() {
  stopWifi();
  disconnectMqtt();
  _enabled = false;
}

bool MqttClient::send(const char* message) {
    if (!_enabled) {
        return false;
    }
    
    // Non-blocking delay: only send if at least 1 second has passed
    unsigned long currentTime = millis();
    if (currentTime - _lastSendTime < 1000) {
        return false; // Too soon, don't send
    }
    
    if(client.connected()==0){
      connectMqtt();
      return false;
    }
    client.publish("esp32/sensor", message);
    client.loop();
    _lastSendTime = currentTime; // Update last send time
    return true;
}

bool MqttClient::startWifi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("M5StickC");
    if (!WiFi.softAPConfig(_local_IP, _gateway, _subnet)) {
        return false;
    }
    M5.delay(500);
    return true;
}

bool MqttClient::stopWifi() {
    WiFi.softAPdisconnect();
    return true;
}

bool MqttClient::connectMqtt() {
    client.setServer(_mqttBrocker, 1883);
    client.connect("esp32");
    return true;
}

bool MqttClient::disconnectMqtt() {
    client.disconnect();
    return true;
}

void MqttClient::toggle() {
    if(!_enabled){
      enable();
      }
    else{
      disable();
    }
}

void MqttClient::loop() {
    if (_enabled) {
      if(client.connected()){
        client.loop();
        }
      else{
        connectMqtt();
      }
    }
}