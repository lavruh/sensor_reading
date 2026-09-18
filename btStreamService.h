#ifndef BT_STREAM_SERVICE_H
#define BT_STREAM_SERVICE_H

#include <Arduino.h>
#include <BluetoothSerial.h>

class btStreamService {
private:
  BluetoothSerial _serial;
  bool _enabled;
  unsigned long _lastSendTime;

  btStreamService();
  btStreamService(const btStreamService&) = delete;
  btStreamService& operator=(const btStreamService&) = delete;

public:
  static btStreamService& getInstance();

  bool enable();
  void disable();
  bool send(const char* message);
  void toggle();

  bool isEnabled() const { return _enabled; }
  bool isConnected();
};

#endif // BT_STREAM_SERVICE_H
