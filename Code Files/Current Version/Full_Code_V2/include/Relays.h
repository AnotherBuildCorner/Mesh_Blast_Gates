#ifndef RELAYS_H
#define RELAYS_H

#include "Primary.h"

// Define the number of relays
#define NUM_RELAYS 2

// Declare external variables
extern const int relayPins[NUM_RELAYS];
extern bool relayStates[NUM_RELAYS];

// Define pushbutton pins
#define PUSHBUTTON1_PIN 2
#define PUSHBUTTON2_PIN 21

// Function prototypes
void Initialize_Relays();
void SetRelayState(int relayIndex, bool state);
void ToggleRelay(int relayIndex);
void ControlRelayWithCollectorState(bool state);
void ControlRelayWithExtraState(bool state);
void ReadPushbuttonsAndControlStates();
void Initialize_Relay_Buttons(); // Update function prototype
void ControlFromIncoming(); // Declare the new function
#endif // RELAYS_H
