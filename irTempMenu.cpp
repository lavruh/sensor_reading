#include <M5Unified.h>

#include "menu.h"
#include "field.h"
#include "Graph.h"
#include "sensors_service.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField body2;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);
extern int displayWidth;

Graph graph(2, 150, displayWidth-4, -20, 200);

void IrTemperatureMenu::loopTask() {
  titleField.show("IR_T *C");
  const SensorReadings& readings = SensorsService::getInstance().readings();
  if (!readings.irTemperatureReady) {
    body1.show("Err");
    return;
  }
  const float temp = readings.irTemperatureC;
  body1.showValue(temp);
  dataToSend = String(temp, 2);
  graph.showHistValue(temp); 
}

void IrTemperatureMenu::onButtonA() {
  typeText(dataToSend);
}
