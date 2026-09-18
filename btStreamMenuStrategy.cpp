#include <M5Unified.h>
#include "btStreamService.h"
#include "field.h"
#include "menu.h"

extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;

void btStreamMenuStrategy::loopTask() {
  btStreamService& stream = btStreamService::getInstance();

  titleField.show("BT Stream");
  body1.show(stream.isEnabled() ? "EN" : "Disabled");
  body2.show(stream.isConnected() ? "Conn" : "Disc");
}

void btStreamMenuStrategy::onButtonA() {
  btStreamService::getInstance().toggle();
}
