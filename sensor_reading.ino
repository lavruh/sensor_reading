#include <M5Unified.h>
#include <M5GFX.h>
#include "field.h"
#include "ble.h"

#include "menu.h"

String dataToSend = "";

M5GFX display;

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

  if(M5.BtnPWR.isHolding()){
    char buf[32];
    snprintf(buf, sizeof(buf), "    %i    ", powerCount);
    titleField.show(buf);
    powerCount= powerCount-1;
    M5.delay(1000);
  }
  if(M5.BtnPWR.isReleased()){
    powerCount = 3;
    titleField.show("");
  }

}

void setup()
{
  Serial.begin(115200);
    auto cfg = M5.config();  
    M5.begin(cfg);  
    M5.Display.setTextSize(1);
    lineH = M5.Display.fontHeight();
    int h = lineH;
    int w = M5.Display.width()/2;

    dateField = Field(M5.Display.width() / 2, 5, h, w);
    menu.subscribe(std::bind(&Field::redraw, &dateField));
    timeField = Field(M5.Display.width() / 2, lineH*2, h, w);
    batField = Field(5, 5, h, w);
    menu.subscribe(std::bind(&Field::redraw, &batField));
    bleField = Field(5, lineH*2, h, w);
    menu.subscribe(std::bind(&Field::redraw, &bleField));
    titleField = TitleField(0, lineH * 5, lineH, M5.Display.width(), &fonts::Font4);
    body1 = TitleField(0, lineH * 9, lineH, M5.Display.width(), &fonts::Font4);
    body2 = TitleField(0, lineH * 13, lineH, M5.Display.width(), &fonts::Font4);

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
