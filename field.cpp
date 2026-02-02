#include <M5Unified.h>
#include "field.h"

void Field::show(String str)
{
  if (str != prevValue)
  {
    M5.Display.drawString(str, x, y);
    prevValue = str;
  }
}
