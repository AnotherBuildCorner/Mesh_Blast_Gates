#include "Current_Sense.h"

void readCurrent() {
    for (int i = 0; i < single_or_dual; i++) {
        powerread[i] = analogRead(powerPins[i]);
        voltages[i] = vcc / ADCmax * powerread[i] * vratio;
        if (currentreadout == 1 || current >= readthresh) {
            Serial.print("V");
            Serial.print(i + 1);
            Serial.print(" ");
            Serial.print(voltages[i]);
            Serial.print(" | ");
        }
    }
    current = calfac * abs(voltages[1] - voltages[0]) / Rsense;

    if (current >= readthresh) {
        Serial.print(" Current ");
        Serial.println(current);
    }
    if (currentreadout == 1) {
        Serial.print(" Current ");
        Serial.println(current);
    }
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
