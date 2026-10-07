#include <M5Unified.h>
#include "menu.h"
#include "field.h"
#include "Graph.h"

#include "sensors_service.h"

extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;
extern String dataToSend;
extern void typeText(String text);
extern int displayWidth;

Graph distanceGraph(2, 150, displayWidth-4, 0, 4000);

void DistanceSensorMenuStrategy::loopTask() {
  titleField.show("Dist mm");
  const SensorReadings& readings = SensorsService::getInstance().readings();
  const uint16_t distance = readings.distanceMm;
  const int rangeStatus = readings.distanceRangeStatus;
  if (!readings.distanceReady) {
    body1.show("Err");
    body2.show("No sensor");
    return;
  }
  switch (rangeStatus)
  {
  case 0:
    // Serial.print("Good");
    status = "                ";
    break;
  case 1:
    status = "Sigma fail";
    break;
  case 2:
    status = "Sign fail";
    break;
  case 7:
    status = "trg fail";
    break;
  default:
    status = "unk ERR";
    break;
  }
  
  body1.showValue(distance);
  dataToSend = String(distance);
  body2.show(status); 
  distanceGraph.showHistValue(distance);
}

void DistanceSensorMenuStrategy::onButtonA() {
  typeText(dataToSend);
}


//  Connecting a VL53L0X/VL53L1X time-of-flight distance sensor unit 
// to the M5StickC Plus2 is achieved via I2C using the Grove port or 
// GPIO pins 0/26 (G0/G26). 
// The StickC Plus2 utilizes the ESP32-PICO-V3-02 and supports UIFlow2, 
// Arduino, and MicroPython for programming. 
// Wiring Connections Connect the VL53L sensor unit to the M5StickC Plus2 Port A (Grove) 
// or directly to the GPIO pins: VL53L VCC \(\rightarrow \) 
// M5StickC Plus2 5VVL53L GND \(\rightarrow \) 

// M5StickC Plus2 GNDVL53L SDA \(\rightarrow \) M5StickC Plus2 G0 (or Port A Yellow)
// VL53L SCL \(\rightarrow \) M5StickC Plus2 G26 (or Port A White) 

// Programming UIFlow2.0: Use the "Unit" section to add the ToF (VL53L) sensor block. 
// Ensure the I2C address is set correctly (default is typically 0x29).
// Arduino: Use the Wire.h library and a VL53L0X library (like the Adafruit VL53L0X library). 
// Initialize with Wire.begin(0, 26).

