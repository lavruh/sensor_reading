#include <M5Unified.h>
#include "BluefruitService.h"
#include "field.h"
#include "menu.h"

extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;

void BluefruitMenuStrategy::loopTask() {
  BluefruitService& service = BluefruitService::getInstance();

  titleField.show("Bluefruit");
  body1.show(service.isEnabled() ? "EN" : "Disabled");
  body2.show(service.isConnected() ? "Conn" : "Disc");
}

void BluefruitMenuStrategy::onButtonA() {
  BluefruitService::getInstance().toggle();
}
