#include "Servos.h"

// Ensure extern variables are declared
const int debounce[NUM_BOARDS] = {100, 100, 100};
const int buttonPins[NUM_BUTTONS] = {2, 21, 22, 23};  // Change as per your setup
const int servoPins[NUM_SERVOS] = {16, 17, 18, 19};   // Change as per your setup
const int ActiveButtons[NUM_BOARDS] = {2, 4, 2};

const int startlimits[NUM_BOARDS][NUM_SERVOS] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}};
const int endlimits[NUM_BOARDS][NUM_SERVOS] = {
    {145, 150, 60, 60},  // Table Saw
    {140, 140, 140, 150}, // Chop Saw
    {150, 160, 60, 60},  // Bandsaw
};

const int *startlimit = startlimits[BoardSel - 1];
const int *endlimit = endlimits[BoardSel - 1];

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

    for (int i = 0; i < NUM_SERVOS; i++) {
        servos[i].detach();
    }
}
