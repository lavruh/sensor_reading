#ifndef BLUEFRUIT_SERVICE_H
#define BLUEFRUIT_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define UART_SERVICE_UUID      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class BluefruitService {
private:
  bool _enabled;
  unsigned long _lastSendTime;
  unsigned long _lastSensorSendTime;
  BLECharacteristic* _pTxCharacteristic;
  BLECharacteristic* _pRxCharacteristic;
  bool _connected;
  bool _accelEnabled;
  bool _gyroEnabled;

  BluefruitService();
  BluefruitService(const BluefruitService&) = delete;
  BluefruitService& operator=(const BluefruitService&) = delete;

public:
  static BluefruitService& getInstance();

  void init(BLEServer* pServer, bool advertiseService = true);
  bool enable();
  void disable();
  bool send(const char* message);
  bool send(float value);
  bool send(float val1, float val2);
  bool sendSensor(const char* label, float value);

  // Bluefruit Controller Protocol "Modules"
  bool sendAccelerometer(float x, float y, float z);
  bool sendGyro(float x, float y, float z);
  bool sendMagnetometer(float x, float y, float z);
  bool sendQuaternion(float x, float y, float z, float w);

  void toggle();
  void handleWrite(BLECharacteristic* pCharacteristic);

  bool isEnabled() const { return _enabled; }
  bool isConnected() const { return _connected; }
  void setConnected(bool connected) { _connected = connected; }

  bool isAccelEnabled() const { return _accelEnabled; }
  bool isGyroEnabled() const { return _gyroEnabled; }
};

#endif // BLUEFRUIT_SERVICE_H
