#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

// Strategy interface for menu screens
class MenuStrategy {
public:
  virtual ~MenuStrategy() = default;
  
  // Display the menu screen
  virtual void loopTask() = 0;
  
  // Handle button A pressed (confirm/select action)
  virtual void onButtonA() = 0;
  
  // Get a name for this menu screen
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
class SetTimeMenuStrategy : public MenuStrategy {
public:
  void loopTask() override;
  void onButtonA() override;
  const char* getName() const override { return "SetTime"; }
};

// Menu manager using Strategy pattern
class Menu {
private:
  MenuStrategy* currentStrategy;
  int currentIndex;
  MenuStrategy* strategies[2];  // Array of available strategies

public:
  Menu();
  ~Menu();
  
  // Switch to next menu (wraps around)
  void nextMenu();
  
  // Get current strategy index
  int getCurrentIndex() const { return currentIndex; }
  
  // Run current menu's loop task
  void loopTask() { if (currentStrategy) currentStrategy->loopTask(); }
  
  // Handle button A press
  void handleButtonA() { if (currentStrategy) currentStrategy->onButtonA(); }
  
  // Get current strategy name
  const char* getCurrentName() const { 
    return currentStrategy ? currentStrategy->getName() : "Unknown"; 
  }
};

#endif // MENU_H
