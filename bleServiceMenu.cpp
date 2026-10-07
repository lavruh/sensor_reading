#include <M5Unified.h>

#include "ble.h"
#include "field.h"
#include "menu.h"
#include "phyphox_service.h"

extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;

void BleServiceMenuStrategy::loopTask() {
  titleField.show("BLE Serv");
  if (!serviceRunning) {
    switch (selectedMode) {
      case Mode::keyboard:
        body1.show("Keyboard");
        break;
      case Mode::phyphox:
        body1.show("Phyphox");
        break;
      case Mode::combined:
        body1.show("Phy+Kbd");
        break;
    }
    body2.show("start ?");
    return;
  }

  switch (selectedMode) {
    case Mode::keyboard:
      body1.show("Keyboard");
      body2.show(isBleConnected ? "Running" : "Waiting");
      break;
    case Mode::phyphox:
      body1.show("Phyphox");
      body2.show(isPhyphoxServiceConnected() ? "Running"
                                             : "Waiting");
      break;
    case Mode::combined:
      body1.show("Phy+Kbd");
      body2.show(isPhyphoxServiceConnected() ? "Running"
                                             : "Waiting");
      break;
  }
}

void BleServiceMenuStrategy::onButtonA() {
  if (serviceRunning) {
    if (selectedMode == Mode::keyboard) {
      if (!disableBleKeyboard()) {
        body2.show("BLE stop failed");
        return;
      }
      stopBluetoothTask();
    } else if (!stopPhyphoxService()) {
      body2.show("BLE stop failed");
      return;
    }
    serviceRunning = false;
  }

  selectedMode = static_cast<Mode>(
      (static_cast<uint8_t>(selectedMode) + 1) % 3);
  loopTask();
}

void BleServiceMenuStrategy::onButtonALongPress() {
  if (serviceRunning) {
    body2.show("active");
    return;
  }

  switch (selectedMode) {
    case Mode::keyboard:
      serviceRunning = startBluetoothTask();
      break;
    case Mode::phyphox:
      serviceRunning = startPhyphoxService();
      break;
    case Mode::combined:
      serviceRunning = startCombinedMode();
      break;
  }
  if (!serviceRunning) {
    body2.show("failed");
    return;
  }
  loopTask();
}

bool BleServiceMenuStrategy::startCombinedMode() {
  if (!startPhyphoxService()) {
    return false;
  }
  if (startBleKeyboardOnServer(getPhyphoxBleServer(), true)) {
    return true;
  }

  if (!stopPhyphoxService()) {
    Serial.println("Failed to clean up after combined BLE startup failure");
  }
  return false;
}
