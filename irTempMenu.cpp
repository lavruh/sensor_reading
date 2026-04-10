#include <M5Unified.h>

#include "Adafruit_MLX90614.h"

#include "menu.h"
#include "field.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField body2;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);

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
  if(!isInit) {
    initSensor();
  }
    float temp = mlx.readObjectTempC();
    body1.showValue(temp);
    // body2.show("*C");

    dataToSend = String(temp, 2);


  
}

void IrTemperatureMenu::onButtonA() {
  typeText(dataToSend);
}

