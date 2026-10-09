#ifndef FIELD_H
#define FIELD_H

#include <Arduino.h>
#include <M5Unified.h>

class Field {
public:
  String prevValue = "";
  int x = 0;
  int y = 0;
  int h = 0;
  int w = 0;

  Field(int x = 0, int y = 0, int h = 0, int w = 0) : prevValue(""), x(x), y(y), h(h), w(w) {}

  void show(String str);
  void update(String str);
  void redraw();
};

class TitleField : public Field {
public:
  const lgfx::IFont* font = nullptr;

  TitleField(int x = 0, int y = 0, int h = 0, int w = 0, const lgfx::IFont* font = &fonts::Font2)
      : Field(x, y, h, w), font(font) {}

  void redraw();
  void show(String str);
  void showValue(float num, String prefix, String suffix);
  void showValue(float num, String prefix);
  void showValue(float num);
  void showValue(int num);
  void update(String str);
};

class LogField : public Field {
public:
  uint8_t maxSymbols;
  uint8_t lineHeight;
  const lgfx::IFont* font;

  LogField(int x = 0, int y = 0, int h = 0, int w = 0,
           uint8_t maxSymbols = 12, uint8_t lineHeight = 16,
           const lgfx::IFont* font = &fonts::FreeSans9pt7b)
      : Field(x, y, h, w),
        maxSymbols(maxSymbols == 0 ? 12 : maxSymbols),
        lineHeight(lineHeight == 0 ? 16 : lineHeight),
        font(font) {}

  void show(String str);
  void update(String str);
  void redraw();
};

#endif // FIELD_H
