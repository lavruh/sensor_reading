#include <M5Unified.h>
#include "field.h"

void Field::update(String str)
{
  M5.Display.fillRect(x, y, w, h, 0x0000);
  M5.Display.drawString(str, x, y);
}

void Field::show(String str)
{
  if (str != prevValue)
  {
    update(str);
    prevValue = str;
  }
}

void TitleField::update(String str)
{
  // M5.Display.fillRect(x, y, w, h, 0x0000);
  int32_t cx = x + w / 2;
  int32_t cy = y + h / 2;
  M5.Display.drawCenterString(str.c_str(), cx, cy, font);
}

void TitleField::show(String str)
{
  if (str != prevValue)
  {
    update(str);
    prevValue = str;
  }
}
