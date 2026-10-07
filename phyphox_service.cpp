#include <M5Unified.h>
#include <BLEDevice.h>
#include <phyphoxBle.h>

#include "ble.h"
#include "phyphox_service.h"
#include "device_name.h"
#include "sensors_service.h"

namespace {
constexpr int PHY_CHANNEL_COUNT = 4;
constexpr uint8_t IR_TEMP_CHANNEL = 1;
constexpr uint8_t DISTANCE_CHANNEL = 2;
constexpr uint8_t CURRENT_CHANNEL = 3;
constexpr uint8_t ACCEL_X_CHANNEL = 4;
constexpr const char* CHANNEL_NAMES[PHY_CHANNEL_COUNT] = {
    "IR", "Distance", "Current", "AccelX"};

PhyphoxBleExperiment::Graph graphs[PHY_CHANNEL_COUNT];
PhyphoxBleExperiment::View motionView;
PhyphoxBleExperiment::View temperatureView;
PhyphoxBleExperiment::View electricalView;

bool experimentConfigured = false;
bool serviceRunning = false;
unsigned long lastDebugPrint = 0;

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

void sendSensorReadings() {
  const SensorReadings& sensorReadings = SensorsService::getInstance().readings();
  float readings[PHY_CHANNEL_COUNT] = {};
  readings[IR_TEMP_CHANNEL - 1] =
      sensorReadings.irTemperatureReady ? sensorReadings.irTemperatureC : 0.0f;
  readings[DISTANCE_CHANNEL - 1] =
      sensorReadings.distanceReady ? sensorReadings.distanceMm : 0.0f;
  readings[CURRENT_CHANNEL - 1] =
      sensorReadings.currentReady ? sensorReadings.currentMa : 0.0f;
  readings[ACCEL_X_CHANNEL - 1] = sensorReadings.imuData.accel.x;

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
    sendSensorReadings();
  }
}
