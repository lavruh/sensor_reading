#include <M5Unified.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "menu.h"
#include "field.h"

// Forward declarations from other files
extern TitleField body1;
extern TitleField titleField;
extern String dataToSend;
extern void typeText(String text);

// DS18B20 sensor on pin G26
#define DS18B20_PIN 25
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);


void IntTempMenuStrategy::loopTask() {
  titleField.show("Temper");

  if(!isInit) {
    sensors.begin();
    isInit = true;
  }

  float temp = 0.0f;
  
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  
  char buf[16];
  if (temp != DEVICE_DISCONNECTED_C) {
    snprintf(buf, sizeof(buf), "   %.2f *C   ", temp);
    dataToSend = String(temp, 2);
  } else {
    snprintf(buf, sizeof(buf), "Sensor Error");
    dataToSend = "No Sensor";
  }
  
  body1.show(buf);
}

void IntTempMenuStrategy::onButtonA() {
  typeText(dataToSend);
}

