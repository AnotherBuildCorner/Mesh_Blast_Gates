#ifndef PRIMARY_H
#define PRIMARY_H

#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Relays.h" // Include Relays.h
#include "Settings.h"
// Define constants

const bool pull = 1; //0 for down 1 for up

#define waittime 1000
#define reboottime 5000
#define rebootpushenable 0
#define NUM_BUTTONS 4 //Max number buttons per board
#define NUM_SERVOS 4 //Max number servos per board
// Define single_or_dual ended sensing
#define single_or_dual 2

// Ensure extern variables are declared
extern int last_active;
extern unsigned long buttonPressStart[NUM_BUTTONS];
extern bool buttonLongPress[NUM_BUTTONS];
extern bool buttonShortPress[NUM_BUTTONS];
extern bool GateStatusLongPress[4];
extern bool GateStatusShortPress[4];
extern bool triggerservos;

// Function prototypes
void setup();
void loop();
void blink_active();
void InitializeWIFI_Serial();
void Initialize_Buttons();
void readButtonPresses();
void resolvePressConflicts(bool longPress[], bool shortPress[], int size);


#endif // PRIMARY_H
