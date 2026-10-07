#include "BluefruitService.h"

#define UART_SERVICE_UUID      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

namespace {
const unsigned long SEND_INTERVAL_MS = 100;
}

class BluefruitCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        BluefruitService::getInstance().handleWrite(pCharacteristic);
    }
};

BluefruitService::BluefruitService()
    : _enabled(false), _lastSendTime(0), _lastSensorSendTime(0), _pTxCharacteristic(nullptr), _pRxCharacteristic(nullptr),
      _connected(false), _accelEnabled(false), _gyroEnabled(false) {
}

BluefruitService& BluefruitService::getInstance() {
  static BluefruitService instance;
  return instance;
}

void BluefruitService::init(BLEServer* pServer, bool advertiseService) {
  BLEService *pService = pServer->createService(UART_SERVICE_UUID);

  _pTxCharacteristic = pService->createCharacteristic(
                        UART_CHARACTERISTIC_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  _pTxCharacteristic->addDescriptor(new BLE2902());

  _pRxCharacteristic = pService->createCharacteristic(
                                          UART_CHARACTERISTIC_RX,
                                          BLECharacteristic::PROPERTY_WRITE |
                                          BLECharacteristic::PROPERTY_WRITE_NR
                                        );

  _pRxCharacteristic->setCallbacks(new BluefruitCallbacks());

  pService->start();

  if (advertiseService) {
    pServer->getAdvertising()->addServiceUUID(UART_SERVICE_UUID);
  }
}

void BluefruitService::handleWrite(BLECharacteristic* pCharacteristic) {
  String value = pCharacteristic->getValue();
  if (value.length() >= 3 && value[0] == '!' && value[1] == 'S') {
    // Sensor command !S<char><byte>
    // App uses !S to control sensors in the Sensor Data view
    char type = value[2];
    bool state = (value[3] == '1');

    if (type == 'a') _accelEnabled = state;
    else if (type == 'g') _gyroEnabled = state;

    Serial.printf("Bluefruit Sensor %c: %s\n", type, state ? "ON" : "OFF");
  } else if (value.length() > 0) {
    Serial.printf("Bluefruit RX: %s\n", value.c_str());
  }
}

bool BluefruitService::enable() {
  _enabled = true;
  _lastSendTime = 0;
  return true;
}

void BluefruitService::disable() {
  _enabled = false;
}

bool BluefruitService::send(const char* message) {
  if (!_enabled || message == nullptr || !_connected || _pTxCharacteristic == nullptr) {
    return false;
  }

  const unsigned long currentTime = millis();
  // Throttle text to 100ms
  if (currentTime - _lastSendTime < SEND_INTERVAL_MS) {
    return false;
  }

  // If sensor data was sent very recently (within 50ms), skip text to avoid interleaving
  if (currentTime - _lastSensorSendTime < 50) {
    return false;
  }

  String msg = String(message);
  if (!msg.endsWith("\n")) {
    msg += "\n";
  }

  _pTxCharacteristic->setValue(msg.c_str());
  _pTxCharacteristic->notify();
  _lastSendTime = currentTime;
  return true;
}

bool BluefruitService::send(float value) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%.2f", value);
  return send(buf);
}

bool BluefruitService::send(float val1, float val2) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f,%.2f", val1, val2);
  return send(buf);
}

bool BluefruitService::sendSensor(const char* label, float value) {
  char buf[48];
  snprintf(buf, sizeof(buf), "%s:%.2f", label, value);
  return send(buf);
}

bool BluefruitService::sendAccelerometer(float x, float y, float z) {
  if (!_enabled || !_connected || _pTxCharacteristic == nullptr) return false;

  uint8_t packet[15];
  packet[0] = '!';
  packet[1] = 'A';
  memcpy(&packet[2], &x, 4);
  memcpy(&packet[6], &y, 4);
  memcpy(&packet[10], &z, 4);

  uint8_t crc = 0;
  for (int i = 1; i < 14; i++) crc += packet[i];
  packet[14] = ~crc;

  _pTxCharacteristic->setValue(packet, 15);
  _pTxCharacteristic->notify();
  _lastSensorSendTime = millis();
  return true;
}

bool BluefruitService::sendGyro(float x, float y, float z) {
  if (!_enabled || !_connected || _pTxCharacteristic == nullptr) return false;

  uint8_t packet[15];
  packet[0] = '!';
  packet[1] = 'G';
  memcpy(&packet[2], &x, 4);
  memcpy(&packet[6], &y, 4);
  memcpy(&packet[10], &z, 4);

  uint8_t crc = 0;
  for (int i = 1; i < 14; i++) crc += packet[i];
  packet[14] = ~crc;

  _pTxCharacteristic->setValue(packet, 15);
  _pTxCharacteristic->notify();
  _lastSensorSendTime = millis();
  return true;
}

bool BluefruitService::sendMagnetometer(float x, float y, float z) {
  if (!_enabled || !_connected || _pTxCharacteristic == nullptr) return false;

  uint8_t packet[15];
  packet[0] = '!';
  packet[1] = 'M';
  memcpy(&packet[2], &x, 4);
  memcpy(&packet[6], &y, 4);
  memcpy(&packet[10], &z, 4);

  uint8_t crc = 0;
  for (int i = 1; i < 14; i++) crc += packet[i];
  packet[14] = ~crc;

  _pTxCharacteristic->setValue(packet, 15);
  _pTxCharacteristic->notify();
  _lastSensorSendTime = millis();
  return true;
}

bool BluefruitService::sendQuaternion(float x, float y, float z, float w) {
  if (!_enabled || !_connected || _pTxCharacteristic == nullptr) return false;

  uint8_t packet[19];
  packet[0] = '!';
  packet[1] = 'Q';
  memcpy(&packet[2], &x, 4);
  memcpy(&packet[6], &y, 4);
  memcpy(&packet[10], &z, 4);
  memcpy(&packet[14], &w, 4);

  uint8_t crc = 0;
  for (int i = 1; i < 18; i++) crc += packet[i];
  packet[18] = ~crc;

  _pTxCharacteristic->setValue(packet, 19);
  _pTxCharacteristic->notify();
  _lastSensorSendTime = millis();
  return true;
}

void BluefruitService::toggle() {
  _enabled = !_enabled;
}
