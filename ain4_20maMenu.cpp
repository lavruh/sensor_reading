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


static Graph ainGraph(2, 150, displayWidth - 4, 0, 25);
void Ain4_20maMenuStrategy::loopTask() {
  titleField.show("4-20mA mA");
  const SensorReadings& readings = SensorsService::getInstance().readings();
  if (!readings.currentReady) {
    body1.show("Err");
    M5.delay(400);
    return;
  }

  body1.showValue(readings.currentMa);
  dataToSend = String(readings.currentMa, 2);
  ainGraph.showHistValue(readings.currentMa);
  M5.delay(400);
}

void Ain4_20maMenuStrategy::onButtonA() {
  typeText(dataToSend);
}
