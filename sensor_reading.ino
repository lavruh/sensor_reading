#include <M5Unified.h>
#include <M5GFX.h>
#include "field.h"
#include "ble.h"

#include <Wire.h>
#include "menu.h"

String dataToSend = "";

M5GFX display;


int displayWidth = 130;
uint16_t lineH = 30;
uint8_t powerCount = 3;


Field dateField = Field();
Field timeField = Field();
Field batField = Field();
Field bleField = Field();
TitleField titleField = TitleField();
TitleField body1 = TitleField();
TitleField body2 = TitleField();

Menu menu = Menu();

void updateClocks(void);
void checkBat(void);

int batteryLevel = 100;


void buttons()
{
  if(M5.BtnA.wasClicked()){
    menu.handleButtonA();
  }

  if(M5.BtnB.wasClicked()){
    menu.nextMenu();
  }

  if(M5.BtnPWR.wasClicked()){
    menu.prevMenu();
  }

  if(M5.BtnPWR.isHolding()){
    M5.Display.clearDisplay(TFT_YELLOW);
    // titleField.show(buf);
    titleField.show("Pwr off:");
    body1.show(String(powerCount));
    powerCount= powerCount-1;
    M5.delay(1000);
  }
  if(M5.BtnPWR.wasReleased()){
    powerCount = 3;
    M5.Display.clearDisplay(TFT_BLACK);
    menu.notifyListeners();
  }

}

void setup()
{
  Serial.begin(115200);
    auto cfg = M5.config();  
    M5.begin(cfg);  
    M5.Display.setTextSize(1);

    int w = M5.Display.width()/2;

    Wire.begin(0, 26);

    int lh = 1;

    dateField = Field(displayWidth / 4 +5, 5, lh, w);
    timeField = Field(displayWidth / 4 +5, 25, lh, w);
    batField = Field(5, 5, lh, w);
    bleField = Field(5, 25, lh, w);
    
    titleField = TitleField(0, 50, lh, displayWidth, &fonts::FreeMono12pt7b);
    body1 = TitleField(0, 80, lh, displayWidth, &fonts::FreeSerif18pt7b);
    body2 = TitleField(0, 110, lh, displayWidth, &fonts::FreeSerif18pt7b);


    menu.subscribe(std::bind(&Field::redraw, &dateField));
    menu.subscribe(std::bind(&Field::redraw, &batField));
    menu.subscribe(std::bind(&Field::redraw, &bleField));
    menu.subscribe(std::bind(&Field::redraw, &timeField));
    menu.subscribe(std::bind(&TitleField::redraw, &titleField));

    M5.Imu.begin();
    xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
    
}

void loop()
{
  M5.update();
  checkBat();
  updateClocks();
  checkBLE();
  buttons();   
  
  menu.loopTask();        
}
