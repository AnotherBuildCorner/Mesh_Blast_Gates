#include "Current_Sense.h"

// Define and initialize the variables
const int powerPins[single_or_dual] = {0, 1}; // Use valid GPIO pins for analog input
float powerread[single_or_dual] = {0};
float voltages[single_or_dual] = {0};
float current = 0;
bool currenttrigger = false;

void readCurrent() {
    for (int i = 0; i < single_or_dual; i++) {
        powerread[i] = analogRead(powerPins[i]);
        voltages[i] = vcc / ADCmax * powerread[i] * vratio;
        if (currentreadout == 1 && current >= readthresh) {
            Serial.print("V");
            Serial.print(i + 1);
            Serial.print(" ");
            Serial.print(voltages[i]);
            Serial.print(" | ");
        }
    }
    current = calfac * abs(voltages[1] - voltages[0]) / Rsense;

    if (current > currentThresh && currenttrigger == false && SenseEnable == 1) {
        if (readout2 == 1) {
            Serial.print("Current: ");
            Serial.print(current);
            Serial.println("Current Trigger");
        }
        currenttrigger = true;
        esp_restart();
    }
}
