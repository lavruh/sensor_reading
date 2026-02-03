#include <M5Unified.h>
#include "menu.h"
#include "field.h"

// Forward declarations from sensor_reading.ino
extern TitleField titleField;
extern Field body1;
extern Field body2;
extern String dataToSend;
extern void showIMU();
extern void setTimeMenu();
extern void setTime();

// Menu implementation
Menu::Menu() : currentIndex(0) {
  // Initialize strategies
  strategies[0] = new IMUMenuStrategy();
  strategies[1] = new SetTimeMenuStrategy();
  
  // Set the current strategy to the first one
  currentStrategy = strategies[0];
}

Menu::~Menu() {
  // Clean up dynamically allocated strategies
  for (int i = 0; i < 2; ++i) {
    delete strategies[i];
  }
}

void Menu::nextMenu() {
  // Move to next menu (wraps around)
  currentIndex = (currentIndex + 1) % 2;
  currentStrategy = strategies[currentIndex];
  
  // Clear display when switching menus
  M5.Display.clear();
}

// Note: Menu::loopTask() wrapper is declared in header and calls current strategy's loopTask().
