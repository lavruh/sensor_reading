#include <M5Unified.h>

#include "Adafruit_MLX90614.h"

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

Graph graph(2, 150, displayWidth-4, -20, 200);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void IrTemperatureMenu::initSensor() {
  if (!mlx.begin(0x5A, &Wire)) {
    body1.show("Err");
    return;
  };
    isInit = true;
}

void IrTemperatureMenu::loopTask() {
  titleField.show("IR_T *C");

  // Ensure Wire is pointing to HAT pins
  Wire.begin(0, 26, 100000UL);

  if(!isInit) {
    initSensor();
  }
    
  float temp = mlx.readObjectTempC();
  body1.showValue(temp);
  dataToSend = String(temp, 2);
  graph.showHistValue(temp);  
  M5.delay(400);
}

void IrTemperatureMenu::onButtonA() {
  typeText(dataToSend);
}

