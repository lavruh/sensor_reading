#include <M5Unified.h>
#include <SparkFun_VL53L1X.h>
#include <DallasTemperature.h>

#include "Adafruit_MLX90614.h"
#include "MODULE_4_20MA.h"
#include "sensors_service.h"

namespace {
constexpr uint8_t IR_TEMP_ADDRESS = 0x5A;
constexpr uint8_t DISTANCE_SDA_PIN = 0;
constexpr uint8_t DISTANCE_SCL_PIN = 26;
constexpr uint8_t CURRENT_SDA_PIN = 32;
constexpr uint8_t CURRENT_SCL_PIN = 33;
constexpr unsigned long I2C_POLL_INTERVAL_MS = 100;
constexpr unsigned long TEMPERATURE_CONVERSION_MS = 750;
constexpr uint8_t DS18B20_PIN = 25;

Adafruit_MLX90614 irSensor;
SFEVL53L1X distanceSensor;
MODULE_4_20MA currentSensor;
OneWire oneWire(DS18B20_PIN);
DallasTemperature internalTemperatureSensor(&oneWire);

bool irSensorReady = false;
bool distanceSensorReady = false;
bool currentSensorReady = false;
bool internalTemperatureSensorReady = false;

void selectI2CBus(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl, 100000UL);
}

bool isI2CDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}
}

SensorsService& SensorsService::getInstance() {
  static SensorsService instance;
  return instance;
}

void SensorsService::begin() {
  if (_initialized) {
    return;
  }

  internalTemperatureSensor.begin();
  internalTemperatureSensorReady = internalTemperatureSensor.getDeviceCount() > 0;
  Serial.printf("DS18B20: %s\n",
                internalTemperatureSensorReady ? "ready" : "not found");
  internalTemperatureSensor.setWaitForConversion(false);
  if (internalTemperatureSensorReady) {
    internalTemperatureSensor.requestTemperatures();
  }
  _lastTemperatureRequest = millis();

  selectI2CBus(DISTANCE_SDA_PIN, DISTANCE_SCL_PIN);
  irSensorReady = irSensor.begin(IR_TEMP_ADDRESS, &Wire);
  Serial.printf("MLX90614: %s\n", irSensorReady ? "ready" : "not found");
  distanceSensor.begin(Wire);
  distanceSensorReady = distanceSensor.checkID();
  Serial.printf("VL53L1X: %s\n", distanceSensorReady ? "ready" : "not found");
  if (distanceSensorReady) {
    distanceSensor.setDistanceModeLong();
    distanceSensor.setTimingBudgetInMs(50);
    distanceSensor.setIntermeasurementPeriod(54);
    distanceSensor.startRanging();
  }

  selectI2CBus(CURRENT_SDA_PIN, CURRENT_SCL_PIN);
  currentSensorReady =
      currentSensor.begin(&Wire, MODULE_4_20MA_ADDR, CURRENT_SDA_PIN, CURRENT_SCL_PIN);
  Serial.printf("4-20mA: %s\n", currentSensorReady ? "ready" : "not found");

  _initialized = true;
}

void SensorsService::update() {
  if (!_initialized) {
    begin();
  }

  const unsigned long now = millis();
  if (internalTemperatureSensorReady &&
      now - _lastTemperatureRequest >= TEMPERATURE_CONVERSION_MS) {
    const float temperature = internalTemperatureSensor.getTempCByIndex(0);
    _readings.internalTemperatureReady = temperature != DEVICE_DISCONNECTED_C;
    if (_readings.internalTemperatureReady) {
      _readings.internalTemperatureC = temperature;
    }
    internalTemperatureSensor.requestTemperatures();
    _lastTemperatureRequest = now;
  }

  if (now - _lastI2CPoll >= I2C_POLL_INTERVAL_MS) {
    pollI2CSensors();
    _lastI2CPoll = now;
  }

  M5.Imu.update();
  _readings.imuData = M5.Imu.getImuData();
}

void SensorsService::pollI2CSensors() {
  selectI2CBus(DISTANCE_SDA_PIN, DISTANCE_SCL_PIN);
  if (irSensorReady) {
    _readings.irTemperatureC = irSensor.readObjectTempC();
    _readings.irTemperatureReady = true;
  }

  if (distanceSensorReady && distanceSensor.checkForDataReady()) {
    _readings.distanceRangeStatus = distanceSensor.getRangeStatus();
    if (_readings.distanceRangeStatus == 0) {
      _readings.distanceMm = distanceSensor.getDistance();
    }
    distanceSensor.clearInterrupt();
    _readings.distanceReady = true;
  }

  selectI2CBus(CURRENT_SDA_PIN, CURRENT_SCL_PIN);
  if (currentSensorReady && isI2CDevicePresent(MODULE_4_20MA_ADDR)) {
    _readings.currentMa = currentSensor.getCurrentValue(0) / 100.0f;
    _readings.currentReady = true;
  } else {
    _readings.currentReady = false;
  }
}

const SensorReadings& SensorsService::readings() const {
  return _readings;
}
