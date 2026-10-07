#ifndef BLE_H
#define BLE_H

#include <Arduino.h>

class BLEServer;

// BLE connection state (defined in ble.cpp)
extern bool isBleConnected;

// Data to send (defined elsewhere)
extern String dataToSend;

// Forward declarations for functions implemented in ble.cpp
void bluetoothTask(void*);
bool startBluetoothTask();
void stopBluetoothTask();
void trackBleConnections();
bool disconnectBleClients(uint32_t timeoutMs = 1500);
bool disableBleKeyboard();
bool startBleKeyboardOnServer(BLEServer* server, bool combinedMode = false);
void resetBleKeyboardState();
void typeText(String text);
void checkBLE();

#endif // BLE_H
