#include <M5Unified.h>
#include "menu.h"
#include "field.h"

// Forward declarations from sensor_reading.ino
extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;
extern String dataToSend;
extern void showIMU();

Menu::Menu() : currentIndex(0) {
  // Initialize strategies
  strategies[0] = new IrTemperatureMenu();
  strategies[1] = new IMUMenuStrategy();
  strategies[2] = new DistanceSensorMenuStrategy();
  strategies[3] = new btStreamMenuStrategy();
  
  // Set the current strategy to the first one
  currentStrategy = strategies[0];
}

Menu::~Menu() {
  // Clean up dynamically allocated strategies
  for (int i = 0; i < strategiesLength(); ++i) {
    delete strategies[i];
  }
}

void Menu::nextMenu() {
  // Move to next menu (wraps around)
  currentIndex = (currentIndex + 1) % strategiesLength();
  currentStrategy = strategies[currentIndex];
  
  // Clear display when switching menus
  M5.Display.clear();

  // Notify listeners about menu change
  notifyListeners();
}

void Menu::prevMenu() {
  // Move to previous menu (wraps around)
  currentIndex = (currentIndex - 1 + strategiesLength()) % strategiesLength();
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
