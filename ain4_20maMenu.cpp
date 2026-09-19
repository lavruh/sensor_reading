#include <M5Unified.h>
#include "MODULE_4_20MA.h"
#include "menu.h"
#include "field.h"
#include "Graph.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField body2;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);
extern int displayWidth;


static Graph ainGraph(2, 150, displayWidth - 4, 0, 25);
static MODULE_4_20MA module_4_20ma;

void Ain4_20maMenuStrategy::initSensor() {
  // Switch Wire to Grove pins 32/33
  Wire.begin(32, 33);

  // Use Wire on pins 32/33 for the 4-20mA module
  if (!module_4_20ma.begin(&Wire, MODULE_4_20MA_ADDR, 32, 33)) {
    body1.show("Err");
    return;
  }
  isInit = true;
}

void Ain4_20maMenuStrategy::loopTask() {
  titleField.show("4-20mA mA");
  Wire.begin(32, 33);

  if (!isInit) {
    initSensor();
  }

  if (isInit) {
    // Read current from channel 0
    uint16_t currentRaw = module_4_20ma.getCurrentValue(0);
    float currentMA = currentRaw / 100.0f; // library returns mA * 100

    body1.showValue(currentMA);
    dataToSend = String(currentMA, 2);
    ainGraph.showHistValue(currentMA);
  }

  M5.delay(400);
}

void Ain4_20maMenuStrategy::onButtonA() {
  typeText(dataToSend);
}
