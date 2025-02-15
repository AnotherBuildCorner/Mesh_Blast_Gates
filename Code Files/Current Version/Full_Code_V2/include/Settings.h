#ifndef SETTINGS_H
#define SETTINGS_H

#include "Primary.h"



#define NUM_GATE_BOARDS 3
#define NUM_PEERS 5  // Total Number of Chips in system
#define NUM_BUTTONS 4 // Max number buttons per board

// Define enums for board labels and types
enum BoardLabelEnum {
    BS = 2,
    TS = 0,
    CS = 1,
    DC = 3,
    Central= 4
};

enum BoardTypeEnum {
    gates,
    collector,
    center,
    remote
};

// Declare settings-related variables
inline BoardLabelEnum BoardSel = Central;
inline BoardTypeEnum BoardTypeArray[] = {gates, gates, gates, collector, center};
inline const char* BoardLabelArray[] = 
{   "TS Board", 
    "CS Board", 
    "BS Board", 
"Collector", "Center Node"};
inline const char GateNames[NUM_GATE_BOARDS][NUM_BUTTONS][20] = {
    {"TableSaw", "Aux", "Gate3", "Gate4"}, 
    {"Planer", "ChopLeft", "ChopRight", "CNC"},
    {"BandSaw", "Sander", "Gate3", "Gate4"},
};
// Declare MAC addresses
inline const uint8_t CentralNodeAddress[6] = {0x8C, 0xBF, 0xEA, 0xCF, 0x7F, 0x00};
inline const uint8_t EndpointAddresses[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Dummy Address for location of Central Node
    {0x8C, 0xBF, 0xEA, 0xCF, 0x82, 0x3C},
    {0x8C, 0xBF, 0xEA, 0xCF, 0x75, 0x44},
    {0xF0, 0xF5, 0xBD, 0x2D, 0x14, 0x4C}, // Bandsaw board
    {0xF0, 0xF5, 0xBD, 0x2C, 0xF9, 0xE0}  // DC V2
};

// Declare constants
constexpr int loopdelay = 30;
constexpr int loopstep = 1;
constexpr int OpenGate = 3;
constexpr int OpenBoard = 1;

// Declare gate names


// Function prototypes

#endif // SETTINGS_H
