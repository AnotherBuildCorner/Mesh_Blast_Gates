#include <Arduino.h>
#include "Primary.h"
#include "Timers.h"
#include "Servos.h"
#include "Mesh_Network.h"
#include "Current_Sense.h"
#include "Relays.h" 
#include "Settings.h" // Include Settings.h

// Function prototype for testing_network
void testing_network();
void endpoint_setup();
void endpoint_main();
void central_node_setup();
void central_node_main();
void collector_node_setup();
void collector_node_main();

void setup() {
    if (BoardTypeArray[BoardSel] == gates) {
        endpoint_setup();
    } else if (BoardTypeArray[BoardSel] == collector) {
        collector_node_setup();
    } else if (BoardTypeArray[BoardSel] == center) {
        central_node_setup();
    }
}

void loop(){
    if (BoardTypeArray[BoardSel] == gates) {
        endpoint_main();
    } else if (BoardTypeArray[BoardSel] == collector) {
        collector_node_main();
    } else if (BoardTypeArray[BoardSel] == center) {
        central_node_main();
    }
}

void endpoint_setup(){
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
}

void endpoint_main(){
    blink_active();
    //testing_network();
    static unsigned long servotimer = 0;
    readButtonPresses();
    //if(millis() > servotimer + 500 || triggerservos == true){ 
      if(triggerservos == true){ 
        updateServoAngles(GateStatusShortPress, GateStatusLongPress);
        write_servo_position();
        servotimer = millis();
        triggerservos = false;
    }
    //readCurrent();
    // Add other loop functionalities here
}

void central_node_setup(){
    // Initialize WiFi and Serial
    InitializeWIFI_Serial();  
    // Launch mesh network
    LaunchCentralNode();
    delay(2000);
    Serial.println("Central Node Setup Complete");
}

void central_node_main(){
    static unsigned long nodetimer = 0;
    blink_active();
    if(new_data_recv == true && millis() > nodetimer + 500){
        push_data();
    }
}

void collector_node_setup(){
    InitializeWIFI_Serial(); 
    Initialize_Relays();
    Initialize_Relay_Buttons();
    LaunchEndpoints();
    delay(2000);
    Serial.println("Collector Node Setup Complete");
}

void collector_node_main(){
    blink_active();
    ControlFromIncoming();
    ReadPushbuttonsAndControlStates();
    reset_timers();
    check_timers();
    // Add other loop functionalities here
}

void testing_network(){
    bool stateprint = 0;
    static unsigned long timertest = 0;
    
    if(millis() > timertest + 10000){ 
        timertest = millis();
        int randomInt = random(0, 9);
        int randomInt2 = random(0, 9);
        mapPressToBinary(randomInt, GateStatusShortPress);
        mapPressToBinary(randomInt2, GateStatusLongPress);
        //resetGateStatusArrays();
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
}

