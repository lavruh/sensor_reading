
#include <M5Unified.h>

#include "menu.h"
#include "field.h"
#include "MqttClient.h"


extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;
extern PubSubClient client;
extern MqttClient& mqtt;


void MqttMenuStrategy::init() {
  
    isInit = true;
}

void MqttMenuStrategy::loopTask() {
  titleField.show("MQTT");

  if(!isInit) {
    init();
  }
  
  if(mqtt.isEnabled()){
    body1.show("EN");
  }
  else{
    body1.show("Disabled");
  }
  if( client.connected()){
    body2.show("Conn");
  }else{
    body2.show("Disc");
  }
}

void MqttMenuStrategy::onButtonA() {
  mqtt.toggle();
}


