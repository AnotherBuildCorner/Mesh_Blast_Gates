#ifndef PRIMARY_H
#define PRIMARY_H

#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

// Define constants
#define BoardSel 2 //1 TS 2 Chop 3 BS, assigns MAC address to the system
#define BoardType 1 //1 For endpoints, 2 for collector, 3 for central

#define SenseEnable false //current sense enable
#define loopdelay 30
#define loopstep 1
#define OpenGate 3
#define OpenBoard 1

const bool pull = 1; //0 for down 1 for up

#define NUM_BUTTONS 4 //Max number buttons per board
#define NUM_SERVOS 4 //Max number servos per board
#define NUM_PEERS 4 //Total Number of Endpoints, and collector boards NOT THE CENTRAL BOARD!!!
#define NUM_BOARDS 3
#define waittime 1000
#define reboottime 5000
#define rebootpushenable 0

// Define single_or_dual ended sensing
#define single_or_dual 2

// Ensure extern variables are declared
extern int last_active;
extern unsigned long buttonPressStart[NUM_BUTTONS];
extern bool buttonLongPress[NUM_BUTTONS];
extern bool buttonShortPress[NUM_BUTTONS];
extern bool GateStatusLongPress[4];
extern bool GateStatusShortPress[4];

// Function prototypes
void setup();
void loop();
void blink_active();
void InitializeWIFI_Serial();
void Initialize_Buttons();
void readButtonPresses();
void resolvePressConflicts(bool longPress[], bool shortPress[], int size);
void TestLED(bool shortPress[], bool longPress[], const int servoPins[]);

#endif // PRIMARY_H
