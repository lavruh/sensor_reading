#include <M5Unified.h>

#include "ble.h"
#include "field.h"
#include "menu.h"
#include "phyphox_service.h"

extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;

void BleServiceMenuStrategy::loopTask() {
  titleField.show("BLE Service");
  if (!serviceRunning) {
    switch (selectedMode) {
      case Mode::keyboard:
        body1.show("Sel: Keyboard");
        break;
      case Mode::phyphox:
        body1.show("Sel: Phyphox");
        break;
      case Mode::combined:
        body1.show("Sel: Combined");
        break;
    }
    body2.show("A select; hold start");
    return;
  }

  switch (selectedMode) {
    case Mode::keyboard:
      body1.show("R: Keyboard");
      body2.show(isBleConnected ? "Kbd connected" : "Waiting");
      break;
    case Mode::phyphox:
      body1.show("R: Phyphox");
      body2.show(isPhyphoxServiceConnected() ? "Phy C"
                                             : "Waiting");
      break;
    case Mode::combined:
      body1.show("R: Combined");
      body2.show(isPhyphoxServiceConnected() ? "Comb C"
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
    body2.show("Service already active");
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
    body2.show("Service start failed");
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
