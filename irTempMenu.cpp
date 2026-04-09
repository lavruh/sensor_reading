#include <M5Unified.h>

#include "Adafruit_MLX90614.h"

#include "menu.h"
#include "field.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void IrTemperatureMenu::initSensor() {
  if (!mlx.begin(0x5A, &Wire)) {
    body1.show("Err init");
    return;
  };
    isInit = true;
}

void IrTemperatureMenu::loopTask() {
  titleField.show("IR Temp");

  if(!isInit) {
    initSensor();
  }

  
    float temp = mlx.readObjectTempC();
    char buf[16];
    snprintf(buf, sizeof(buf), "   %.2f *C   ", temp);

    dataToSend = String(temp, 2);
    body1.show(buf);

  
}

void IrTemperatureMenu::onButtonA() {
  typeText(dataToSend);
}

