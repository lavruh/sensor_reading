#ifndef BLE_H
#define BLE_H

#include <Arduino.h>

// BLE connection state (defined in ble.cpp)
extern bool isBleConnected;

// Data to send (defined elsewhere)
extern String dataToSend;

// Forward declarations for functions implemented in ble.cpp
void bluetoothTask(void*);
void typeText(String text);
void checkBLE();

#endif // BLE_H
