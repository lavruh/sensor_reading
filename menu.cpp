#include <M5Unified.h>
#include "menu.h"
#include "field.h"

// Forward declarations from sensor_reading.ino
extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;
extern String dataToSend;
extern void showIMU();
extern void setTimeMenu();
extern void setTime();

// Menu implementation
Menu::Menu() : currentIndex(0) {
  // Initialize strategies
  strategies[0] = new IMUMenuStrategy();
  strategies[1] = new IrTemperatureMenu();
  strategies[2] = new IntTempMenuStrategy();
  strategies[3] = new DistanceSensorMenuStrategy();
  strategies[4] = new SetTimeMenuStrategy();
  
  // Set the current strategy to the first one
  currentStrategy = strategies[0];
}

Menu::~Menu() {
  // Clean up dynamically allocated strategies
  for (int i = 0; i < 5; ++i) {
    delete strategies[i];
  }
}

void Menu::nextMenu() {
  // Move to next menu (wraps around)
  currentIndex = (currentIndex + 1) % 5;
  currentStrategy = strategies[currentIndex];
  
  // Clear display when switching menus
  M5.Display.clear();

  // Notify listeners about menu change
  notifyListeners();
}

void Menu::prevMenu() {
  // Move to previous menu (wraps around)
  currentIndex = (currentIndex - 1 + 5) % 5;
  currentStrategy = strategies[currentIndex];
  
  // Clear display when switching menus
  M5.Display.clear();

  // Notify listeners about menu change
  notifyListeners();
}

// Note: Menu::loopTask() wrapper is declared in header and calls current strategy's loopTask().

bool Menu::subscribe(const std::function<void()>& callback) {
  if (_subscriberCount < MAX_SUBSCRIBERS && callback) {
    _subscribers[_subscriberCount++] = callback;
    return true;
  }
  return false;
}

void Menu::notifyListeners() {
  for (int i = 0; i < _subscriberCount; ++i) {
    if (_subscribers[i]) {
      _subscribers[i]();
    }
  }
}
