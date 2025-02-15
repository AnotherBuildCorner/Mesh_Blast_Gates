#ifndef SERVOS_H
#define SERVOS_H

#include "Primary.h"
#include <ESP32Servo.h>
#include "Settings.h"

// Declare external variables
extern const int debounce[NUM_GATE_BOARDS]; // Updated constant name
extern const int buttonPins[NUM_BUTTONS];
extern const int servoPins[NUM_SERVOS];
extern const int ActiveButtons[NUM_GATE_BOARDS]; // Updated constant name
extern const int startlimits[NUM_GATE_BOARDS][NUM_SERVOS]; // Updated constant name
extern const int endlimits[NUM_GATE_BOARDS][NUM_SERVOS]; // Updated constant name
extern const int *startlimit;
extern const int *endlimit;
extern Servo servos[NUM_SERVOS];
extern int angle[NUM_SERVOS];
extern int pastangle[NUM_SERVOS];
extern bool gateflag; // Declare gateflag here

// Function prototypes
void Initialize_Servos();
void write_servo_position();
void updateServoAngles(bool shortPress[], bool longPress[]);
void PrintCommandedAngles();
#endif // SERVOS_H
