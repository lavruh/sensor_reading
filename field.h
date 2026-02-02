#ifndef FIELD_H
#define FIELD_H

#include <Arduino.h>

class Field {
public:
  String prevValue = "";
  int x = 0;
  int y = 0;

  Field(int x = 0, int y = 0) : prevValue(""), x(x), y(y) {}

  void show(String str);
};

#endif // FIELD_H
