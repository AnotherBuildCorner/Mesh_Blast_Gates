#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include "Mesh_Network.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Function prototypes
void initializeScreen();
void updateScreen(const char* message);
bool parseBoardData(const Board_Data& data, int& shortIndex, int& shortValue, int& longIndex, int& longValue);
void convertAndSelectGateName(const Board_Data& data, char* shortGateName, char* longGateName);
void startCountdownTimer(const Board_Data& data);
void displayOLED(const char* shortGateName, const char* longGateName, int countdownTime);
bool screenhandler(bool reset_countdown);
#endif // SCREEN_H
