#ifndef LEDS_H
#define LEDS_H

#include "Settings.h"

void RunLED(int LED_Pin, bool active, bool mode);
void checkstates();
void RunLEDs(bool mode);
void initializeLEDs();

#endif // LEDS_H
