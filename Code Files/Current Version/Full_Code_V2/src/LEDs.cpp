#include "LEDs.h"
#include "primary.h"
#include <Arduino.h>
#include "mesh_network.h"
const int LED_pins[3] = {17,18,19};
int states[3] ={0};


void RunLED(int LED_Pin, bool active, bool mode){
    static int light = 0;
    static unsigned long t1 = millis();
    static int interval = 5;
if (mode == true){
  if(millis() > t1 + 50){
    light += interval;
    if(light >= 255){
      light = 255;
      interval = interval * -1;
    }
    if(light <= 0){
      light = 0;
      interval = interval * -1;
    }
    if(active == false){
        light = 0;
    }
    analogWrite(LED_Pin,light);
    }
  }
else{
    if(active == true){
        digitalWrite(LED_Pin,HIGH);
    }
    else{
        digitalWrite(LED_Pin,LOW);
    }
}
}


void checkstates(){
    static int Longs = 0;
    static int Shorts = 0;
    for(int i = 0; i < NUM_PEERS; i++){
        Longs += BoardData.LongPress[i];
        Shorts += BoardData.ShortPress[i];
    }
    if(Longs > 0){
        states[0] = 1;
    }
    else{
        states[0] = 0;
    }
    
    if(Shorts > 0){
        states[1] = 1;
    }
    else{
        states[1] = 0;
    }
}

void RunLEDs(bool mode){
    for(int i = 0; i < 3; i++){
        RunLED(LED_pins[i],states[i],mode);
    }
}

void initializeLEDs(){
    for(int i = 0; i < 3; i++){
        pinMode(LED_pins[i],OUTPUT);
    }
}