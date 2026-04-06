#include <M5Unified.h>
#include "field.h"

extern int batteryLevel;
extern Field batField;

void checkBat()
{
  batteryLevel = M5.Power.getBatteryLevel();

  char buf[16];
  snprintf( buf, 16, "B: %03d", batteryLevel);
  batField.show( String(buf) );
}
