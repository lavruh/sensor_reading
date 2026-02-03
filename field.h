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
};

class TitleField : public Field {
public:
  const lgfx::IFont* font = nullptr;

  TitleField(int x = 0, int y = 0, int h = 0, int w = 0, const lgfx::IFont* font = &fonts::Font2)
      : Field(x, y, h, w), font(font) {}

  void show(String str);
  void update(String str);
};
#endif // FIELD_H
