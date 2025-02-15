#include "Servos.h"
#include "Current_Sense.h"
#include "Settings.h"
#include "mesh_network.h"
// Define the gateflag variable
bool gateflag = true;

// Ensure extern variables are declared
const int debounce[NUM_GATE_BOARDS] = {100, 100, 100}; // Updated constant name
const int buttonPins[NUM_BUTTONS] = {2, 21, 22, 23};  // Change as per your setup
const int servoPins[NUM_SERVOS] = {16, 17, 18, 19};   // Change as per your setup
const int ActiveButtons[NUM_GATE_BOARDS] = {2, 4, 2}; // Updated constant name

const int startlimits[NUM_GATE_BOARDS][NUM_SERVOS] = { // Updated constant name
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}};
const int endlimits[NUM_GATE_BOARDS][NUM_SERVOS] = { // Updated constant name
    {145, 150, 60, 60},  // Table Saw
    {140, 140, 140, 150}, // Chop Saw
    {150, 160, 60, 60},  // Bandsaw
};

const int *startlimit = startlimits[BoardSel];
const int *endlimit = endlimits[BoardSel];

Servo servos[NUM_SERVOS];
int angle[NUM_SERVOS];
int pastangle[NUM_SERVOS];

void Initialize_Servos() {
    for (int i = 0; i < NUM_SERVOS; i++) {
        servos[i].attach(servoPins[i]);
        angle[i] = startlimit[i];
        pastangle[i] = angle[i];
        servos[i].write(angle[i]);
        delay(1000);
    }

    //for (int i = 0; i < NUM_SERVOS; i++) {
      //  servos[i].detach();}
    
}

void write_servo_position() {
    static bool newdata = false;
    static bool targetreached = false;
    for(int k = 0; k < NUM_SERVOS; k++){   
        if(angle[k] != pastangle[k]){
            newdata=true;
        }}
        if(newdata==true){
            PrintCommandedAngles();
            PrintLocalGateArrays();
            newdata = false;
        }
        

    for (int j = 0; j < NUM_SERVOS; j++) {
        if (pastangle[j] < angle[j]) {
            //servos[j].attach(servoPins[j]);
            Serial.print("Gate ");
            Serial.print(GateNames[BoardSel][j]);
            Serial.println(" Opening");
            gateflag = true;
            for (int i = pastangle[j]; i <= angle[j]; i++) {
                readCurrent();
                // Serial.println(i);
                servos[j].write(i);
                delay(loopdelay);
            }
            //servos[j].detach();
            pastangle[j] = angle[j];
        }

        if (pastangle[j] > angle[j] && gateflag == true) {
            //servos[j].attach(servoPins[j]);
            Serial.print("Gate ");
            Serial.print(GateNames[BoardSel][j]);
            Serial.println(" Closing");
            for (int i = pastangle[j]; i >= angle[j]; i--) {
                readCurrent();
                // Serial.println(i);
                servos[j].write(i);
                delay(loopdelay);
            }
            //servos[j].detach();
            pastangle[j] = angle[j];
        }
    }
}

void updateServoAngles(bool shortPress[], bool longPress[]) {
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (shortPress[i] || longPress[i]) {
            angle[i] = endlimit[i];
        } else {
            angle[i] = startlimit[i];
        }
    }
}

void PrintCommandedAngles(){
    Serial.print("Commanded Angles | (");
    for(int i = 0; i < NUM_SERVOS; i++){
        Serial.print(angle[i]);
        Serial.print(", ");
    }
    Serial.println(")");
}