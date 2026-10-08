#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <functional>

// Strategy interface for menu screens
class MenuStrategy {
public:
  virtual ~MenuStrategy() = default;
  virtual void loopTask() = 0;
  virtual void onButtonA() = 0;
  virtual void onButtonALongPress() {}
  virtual bool onButtonB() { return false; }
  virtual bool onButtonPower() { return false; }
  virtual bool blocksPowerOffScreen() const { return false; }
  virtual const char* getName() const = 0;
};

class SetTimeMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  void onButtonALongPress() override;
  bool onButtonB() override;
  bool onButtonPower() override;
  bool blocksPowerOffScreen() const override { return editing; }
  const char* getName() const override { return "Set Time"; }

private:
  enum class Step : uint8_t { year, month, day, hours, minutes, seconds };

  bool editing = false;
  bool updated = false;
  bool cancelled = false;
  const char* rtcError = nullptr;
  Step step = Step::year;
  uint16_t year = 0;
  uint8_t month = 1;
  uint8_t day = 1;
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;

  void adjustValue(int8_t direction);
  void beginEditing();
  void confirmStep();
  void showCurrentStep();
};

// Concrete strategy: IMU display screen
class IMUMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "IMU"; }
};

// Concrete strategy: Internal Temperature screen
class IntTempMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "IntTemp"; }
};

// Concrete strategy: Distance Sensor screen
class DistanceSensorMenuStrategy : public MenuStrategy {
public:
  String status = "";
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "Distance"; }
};

// Concrete strategy: IR Temperature screen
class IrTemperatureMenu : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "IRTemperature"; }
};

class MqttMenuStrategy : public MenuStrategy {
public:
  bool isInit = false;

  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "MQTT"; }
  void init();
};

class Ain4_20maMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "4-20mA"; }
};

class BleServiceMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  void onButtonALongPress() override;
  bool startCombinedMode();
  const char* getName() const override { return "BLE Service"; }

private:
  enum class Mode : uint8_t {
    keyboard,
    phyphox,
    combined
  };

  Mode selectedMode = Mode::combined;
  bool serviceRunning = false;
};

// Menu manager using Strategy pattern
class Menu {
private:
  MenuStrategy* currentStrategy;
  int currentIndex;
  MenuStrategy* strategies[6];  // Array of available strategies
  static const int MAX_SUBSCRIBERS = 8;
  // subscriber callbacks
  std::function<void()> _subscribers[MAX_SUBSCRIBERS];
  int _subscriberCount = 0;

public:
  Menu();
  ~Menu();
  
  // Switch to next menu (wraps around)
  void nextMenu();
  
  // Switch to previous menu (wraps around)
  void prevMenu();
  
  // Get current strategy index
  int getCurrentIndex() const { return currentIndex; }
  
  // Get the number of available strategies
  int strategiesLength() const { return sizeof(strategies) / sizeof(strategies[0]); }
  
  // Run current menu's loop task
  void loopTask() { if (currentStrategy) currentStrategy->loopTask(); }
  
  // Handle button A press
  void handleButtonA() { if (currentStrategy) currentStrategy->onButtonA(); }
  void handleButtonALongPress() {
    if (currentStrategy) currentStrategy->onButtonALongPress();
  }
  void handleButtonB() {
    if (!currentStrategy || !currentStrategy->onButtonB()) nextMenu();
  }
  void handleButtonPower() {
    if (!currentStrategy || !currentStrategy->onButtonPower()) prevMenu();
  }
  bool blocksPowerOffScreen() const {
    return currentStrategy && currentStrategy->blocksPowerOffScreen();
  }

  // Subscribe a callback; callbacks have signature void()
  bool subscribe(const std::function<void()>& callback);

  // Notify all subscribers
  void notifyListeners();
  
  // Get current strategy name
  const char* getCurrentName() const { 
    return currentStrategy ? currentStrategy->getName() : "Unknown"; 
  }
};

#endif // MENU_H
