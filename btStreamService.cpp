#include "btStreamService.h"
#include "device_name.h"

namespace {
const unsigned long SEND_INTERVAL_MS = 1000;
}

btStreamService::btStreamService()
    : _enabled(false), _lastSendTime(0) {
}

btStreamService& btStreamService::getInstance() {
  static btStreamService instance;
  return instance;
}

bool btStreamService::enable() {
  if (_enabled) {
    return true;
  }

  if (!_serial.begin(DEVICE_NAME)) {
    return false;
  }

  _enabled = true;
  _lastSendTime = 0;
  return true;
}

void btStreamService::disable() {
  if (!_enabled) {
    return;
  }

  _serial.end();
  _enabled = false;
}

bool btStreamService::send(const char* message) {
  if (!_enabled || message == nullptr || !_serial.hasClient()) {
    return false;
  }

  const unsigned long currentTime = millis();
  if (currentTime - _lastSendTime < SEND_INTERVAL_MS) {
    return false;
  }

  _serial.println(message);
  _lastSendTime = currentTime;
  return true;
}

void btStreamService::toggle() {
  if (_enabled) {
    disable();
  } else {
    enable();
  }
}

bool btStreamService::isConnected() {
  return _enabled && _serial.hasClient();
}
