#include "menu.h"
#include "field.h"
#include "sensors_service.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);

void IntTempMenuStrategy::loopTask() {
  titleField.show("T *C");
  const SensorReadings& readings = SensorsService::getInstance().readings();
  if (readings.internalTemperatureReady) {
    body1.showValue(readings.internalTemperatureC);
    dataToSend = String(readings.internalTemperatureC, 2);
  } else {
    body1.show("Err");
    dataToSend = "No Sensor";
  }
}

void IntTempMenuStrategy::onButtonA() {
  typeText(dataToSend);
}
