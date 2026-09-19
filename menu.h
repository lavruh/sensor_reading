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
  virtual const char* getName() const = 0;
};

// Concrete strategy: IMU display screen
class IMUMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "IMU"; }
};

// Concrete strategy: Set Time menu screen
// class SetTimeMenuStrategy : public MenuStrategy {
// public:
//   void loopTask() override;
//   void onButtonA() override;
//   const char* getName() const override { return "SetTime"; }
// };

// Concrete strategy: Internal Temperature screen
class IntTempMenuStrategy : public MenuStrategy {
public:
  bool isInit = false;
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "IntTemp"; }
};

// Concrete strategy: Distance Sensor screen
class DistanceSensorMenuStrategy : public MenuStrategy {
public:
  String status = "";
  bool isInit = false;
  void loopTask() override;
  void onButtonA() override;
  void initSensor();
  const char* getName() const override { return "Distance"; }
};

// Concrete strategy: IR Temperature screen
class IrTemperatureMenu : public MenuStrategy {
public:
  bool isInit = false;
  void loopTask() override;
  void onButtonA() override;
  void initSensor();
  const char* getName() const override { return "IRTemperature"; }
};

class btStreamMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "BT Stream"; }
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
  bool isInit = false;
  void loopTask() override;
  void onButtonA() override;
  void initSensor();
  const char* getName() const override { return "4-20mA"; }
};

// Menu manager using Strategy pattern
class Menu {
private:
  MenuStrategy* currentStrategy;
  int currentIndex;
  // int strategyCount = 6;
  MenuStrategy* strategies[5];  // Array of available strategies
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
