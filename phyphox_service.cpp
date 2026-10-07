#include <M5Unified.h>
#include <SparkFun_VL53L1X.h>
#include <BLEDevice.h>
#include <phyphoxBle.h>

#include "Adafruit_MLX90614.h"
#include "MODULE_4_20MA.h"
#include "ble.h"
#include "phyphox_service.h"
#include "device_name.h"

namespace {
constexpr int PHY_CHANNEL_COUNT = 4;
constexpr uint8_t IR_TEMP_CHANNEL = 1;
constexpr uint8_t DISTANCE_CHANNEL = 2;
constexpr uint8_t CURRENT_CHANNEL = 3;
constexpr uint8_t ACCEL_X_CHANNEL = 4;
constexpr const char* CHANNEL_NAMES[PHY_CHANNEL_COUNT] = {
    "IR", "Distance", "Current", "AccelX"};

Adafruit_MLX90614 irSensor;
SFEVL53L1X distanceSensor;
MODULE_4_20MA currentSensor;

PhyphoxBleExperiment::Graph graphs[PHY_CHANNEL_COUNT];
PhyphoxBleExperiment::View motionView;
PhyphoxBleExperiment::View temperatureView;
PhyphoxBleExperiment::View electricalView;

bool irSensorReady = false;
bool distanceSensorReady = false;
bool currentSensorReady = false;
bool experimentConfigured = false;
bool serviceRunning = false;
float distanceMm = 0.0f;
unsigned long lastDebugPrint = 0;

void selectI2CBus(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl, 100000UL);
}

bool isI2CDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void configureGraph(
    PhyphoxBleExperiment::Graph& graph,
    const char* label,
    const char* unit,
    int channel) {
  graph.setLabel(label);
  graph.setUnitX("s");
  graph.setUnitY(unit);
  graph.setLabelX("Time");
  graph.setLabelY(label);
  graph.setChannel(0, channel);
}

void configureExperiment() {
  PhyphoxBleExperiment experiment;
  experiment.setTitle("M5StickC Plus2 Sensors");
  experiment.setCategory("Microcontroller Experiments");
  experiment.setDescription(
      "Streams IR temperature, distance, current, and accelerometer X.");

  PhyphoxBleExperiment::numberOfChannels = PHY_CHANNEL_COUNT;
  configureGraph(graphs[ACCEL_X_CHANNEL - 1], "Accel X", "g", ACCEL_X_CHANNEL);
  configureGraph(graphs[IR_TEMP_CHANNEL - 1], "Object Temp", "°C", IR_TEMP_CHANNEL);
  configureGraph(graphs[DISTANCE_CHANNEL - 1], "Distance", "mm", DISTANCE_CHANNEL);
  configureGraph(graphs[CURRENT_CHANNEL - 1], "Current", "mA", CURRENT_CHANNEL);

  motionView.setLabel("Motion");
  motionView.addElement(graphs[ACCEL_X_CHANNEL - 1]);
  temperatureView.setLabel("Temperature and Distance");
  temperatureView.addElement(graphs[IR_TEMP_CHANNEL - 1]);
  temperatureView.addElement(graphs[DISTANCE_CHANNEL - 1]);
  electricalView.setLabel("Electrical");
  electricalView.addElement(graphs[CURRENT_CHANNEL - 1]);

  experiment.addView(motionView);
  experiment.addView(temperatureView);
  experiment.addView(electricalView);
  PhyphoxBLE::addExperiment(experiment);
}

void initializeSensors() {
  selectI2CBus(0, 26);
  irSensorReady = irSensor.begin(0x5A, &Wire);
  distanceSensor.begin(Wire);
  distanceSensorReady = distanceSensor.checkID();
  Serial.printf("Phy VL53L1X: %s\n", distanceSensorReady ? "ready" : "not found");
  if (distanceSensorReady) {
    distanceSensor.setDistanceModeLong();
    distanceSensor.setTimingBudgetInMs(50);
    distanceSensor.setIntermeasurementPeriod(54);
    distanceSensor.startRanging();
  }

  selectI2CBus(32, 33);
  currentSensorReady = currentSensor.begin(&Wire, MODULE_4_20MA_ADDR, 32, 33);
}

void readAndSendSensors() {
  float readings[PHY_CHANNEL_COUNT] = {};

  selectI2CBus(0, 26);
  readings[IR_TEMP_CHANNEL - 1] =
      irSensorReady ? irSensor.readObjectTempC() : 0.0f;

  if (distanceSensorReady && distanceSensor.checkForDataReady()) {
    const uint16_t rangeStatus = distanceSensor.getRangeStatus();
    distanceMm = rangeStatus == 0 ? distanceSensor.getDistance() : 0.0f;
    distanceSensor.clearInterrupt();
  }
  readings[DISTANCE_CHANNEL - 1] = distanceMm;

  selectI2CBus(32, 33);
  readings[CURRENT_CHANNEL - 1] =
      currentSensorReady && isI2CDevicePresent(MODULE_4_20MA_ADDR)
          ? currentSensor.getCurrentValue(0) / 100.0f
          : 0.0f;

  M5.Imu.update();
  const auto imuData = M5.Imu.getImuData();
  readings[ACCEL_X_CHANNEL - 1] = imuData.accel.x;

  const unsigned long now = millis();
  if (now - lastDebugPrint >= 250) {
    Serial.print("Phy readings:");
    for (int i = 0; i < PHY_CHANNEL_COUNT; ++i) {
      Serial.printf(" %s=%.2f", CHANNEL_NAMES[i], readings[i]);
    }
    Serial.println();
    lastDebugPrint = now;
  }

  if (PhyphoxBLE::currentConnections > 0) {
    PhyphoxBLE::write(readings, PHY_CHANNEL_COUNT);
  }
}
}  // namespace

bool startPhyphoxService() {
  if (serviceRunning) {
    return true;
  }
  if (BLEDevice::getInitialized()) {
    Serial.println("Cannot start Phyphox while another BLE service is active");
    return false;
  }

  trackBleConnections();
  if (!experimentConfigured) {
    configureExperiment();
    experimentConfigured = true;
  }
  PhyphoxBLE::start(DEVICE_NAME);
  initializeSensors();
  serviceRunning = true;
  return true;
}

bool stopPhyphoxService() {
  if (!serviceRunning) {
    return true;
  }
  if (!disconnectBleClients()) {
    return false;
  }
  if (BLEDevice::getInitialized()) {
    BLEDevice::deinit(false);
  }
  resetBleKeyboardState();
  PhyphoxBLE::currentConnections = 0;
  PhyphoxBLE::isSubscribed = false;
  serviceRunning = false;
  return true;
}

bool isPhyphoxServiceConnected() {
  return PhyphoxBLE::currentConnections > 0;
}

BLEServer* getPhyphoxBleServer() {
  return PhyphoxBLE::getServer();
}

void loopPhyphoxService() {
  if (serviceRunning) {
    readAndSendSensors();
  }
}
