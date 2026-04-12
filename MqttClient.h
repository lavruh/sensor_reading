#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <PubSubClient.h>



class MqttClient {
private:
    bool _enabled;
    unsigned long _lastSendTime;

    MqttClient();
    MqttClient(const MqttClient&) = delete;
    MqttClient& operator=(const MqttClient&) = delete;
    bool startWifi();
    bool stopWifi();
    bool connectMqtt();
    bool disconnectMqtt();

public:
    static MqttClient& getInstance();
    
    bool enable();
    void disable();
    bool send(const char* message);
    void toggle();

    bool isEnabled() const { return _enabled; }
    void loop();
};

#endif // MQTTCLIENT_H