#include <M5Unified.h>
#include "field.h"

void Field::update(String str)
{
  M5.Display.drawString(str, x, y, &fonts::FreeSans9pt7b);
}

void Field::show(String str)
{
  if (str != prevValue)
  {
    update(str);
    prevValue = str;
  }
}

void Field::redraw()
{
  update(prevValue);
}

void TitleField::update(String str)
{
  // M5.Display.fillRect(x, y, w, h, 0x0050);
  int32_t cx = x + w / 2;
  int32_t cy = y + h / 2;
  M5.Display.drawCenterString(str.c_str(), cx, cy, font);
}

void TitleField::show(String str)
{
  String s = "    " + str + "    ";
  if (s != prevValue)
  {
    update(s);
    prevValue = s;
  }
}


  void TitleField::showValue(float num, String prefix = "", String suffix = "")
  {
    char buf[16];
    snprintf(buf, sizeof(buf), "%s%.1f%s", prefix.c_str(), num, suffix.c_str());
    show(buf);
  }

  void TitleField::showValue(float num, String prefix = "")
  {
    char buf[16];
    snprintf(buf, sizeof(buf), "%s%.1f", prefix.c_str(), num);
    show(buf);
  }

  void TitleField::showValue(float num)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", num);
    show(buf);
  }

  void TitleField::showValue(int num)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", num);
    show(buf);
  }
