#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include <M5Unified.h>

struct SensorReadings {
  float internalTemperatureC = 0.0f;
  bool internalTemperatureReady = false;
  float irTemperatureC = 0.0f;
  bool irTemperatureReady = false;
  uint16_t distanceMm = 0;
  int distanceRangeStatus = -1;
  bool distanceReady = false;
  float currentMa = 0.0f;
  bool currentReady = false;
  m5::imu_data_t imuData{};
};

class SensorsService {
public:
  static SensorsService& getInstance();
  void begin();
  void update();
  const SensorReadings& readings() const;

private:
  SensorsService() = default;
  void pollI2CSensors();

  SensorReadings _readings;
  bool _initialized = false;
  unsigned long _lastI2CPoll = 0;
  unsigned long _lastTemperatureRequest = 0;
};

#endif
