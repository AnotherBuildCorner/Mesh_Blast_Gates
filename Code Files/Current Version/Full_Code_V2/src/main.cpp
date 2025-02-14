#include <Arduino.h>
#include "Primary.h"
#include "Timers.h"
#include "Servos.h"
#include "Mesh_Network.h"
#include "Current_Sense.h"

void setup() {
    // Initialize WiFi and Serial
    InitializeWIFI_Serial();
    
    // Initialize buttons
    Initialize_Buttons();
    
    // Initialize servos
    Initialize_Servos();
    
    // Launch mesh network
    LaunchEndpoints();
    delay(2000);
    
    Serial.println("Setup Complete");

    startup_push();
}

void loop() {
    blink_active();
    static unsigned long timertest = 0;
    if(millis() > timertest + 10000){ 
      timertest = millis();
      int randomInt = random(0, 16);
      int randomInt2 = random(0, 16);
      mapPressToBinary(randomInt, GateStatusShortPress);
      mapPressToBinary(randomInt2, GateStatusLongPress);
      resetGateStatusArrays();
      resolvePressConflicts(buttonLongPress, buttonShortPress, NUM_BUTTONS);
      sendGateStatusToCentralNode(GateStatusLongPress, GateStatusShortPress);

      Serial.print("Long Press States | (");
  for(int i = 0; i < NUM_PEERS; i++){
    Serial.print(BoardData.LongPress[i]);
    Serial.print(", ");
  }
  Serial.println(")");
  Serial.print("Short Press States | (");
  for(int i = 0; i < NUM_PEERS; i++){
    Serial.print(BoardData.ShortPress[i]);
    Serial.print(", ");
  }
  Serial.println(")");
    }
    //readCurrent();
    // Add other loop functionalities here
}

