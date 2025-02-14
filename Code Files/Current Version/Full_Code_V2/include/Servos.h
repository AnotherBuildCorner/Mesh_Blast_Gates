#ifndef SERVOS_H
#define SERVOS_H

#include "Primary.h"
#include <ESP32Servo.h>

// Declare external variables
extern const int debounce[NUM_BOARDS];
extern const int buttonPins[NUM_BUTTONS];
extern const int servoPins[NUM_SERVOS];
extern const int ActiveButtons[NUM_BOARDS];
extern const int startlimits[NUM_BOARDS][NUM_SERVOS];
extern const int endlimits[NUM_BOARDS][NUM_SERVOS];
extern const int *startlimit;
extern const int *endlimit;
extern Servo servos[NUM_SERVOS];
extern int angle[NUM_SERVOS];
extern int pastangle[NUM_SERVOS];

// Function prototypes
void Initialize_Servos();

#endif // SERVOS_H
