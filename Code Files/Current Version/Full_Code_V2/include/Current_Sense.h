#ifndef CURRENT_SENSE_H
#define CURRENT_SENSE_H

#include "Primary.h"

#define SenseEnable false

#define vss 5 // 6.1?
#define vcc 3.3
#define vratio 2
#define Rsense 0.47
#define calfac 0.5
#define ADCmax 3000.0
#define currentreadout 0
#define readthresh 0.5
#define readout2 0
#define currentThresh 1.5

extern const int powerPins[single_or_dual];
extern float powerread[single_or_dual];
extern float voltages[single_or_dual];
extern float current;
extern bool currenttrigger;

void readCurrent();

#endif // CURRENT_SENSE_H
