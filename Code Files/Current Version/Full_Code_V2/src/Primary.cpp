#include "Primary.h"
#include "Servos.h" // Include Servos.h to access necessary variables
#include "Mesh_Network.h" // Include Mesh_Network.h to access necessary variables

// Define last_active
int last_active = 10;

// Ensure extern variables are declared
extern const int debounce[NUM_GATE_BOARDS]; // Updated constant name
extern const int buttonPins[NUM_BUTTONS];
extern const int servoPins[NUM_SERVOS];
extern const int ActiveButtons[NUM_GATE_BOARDS]; // Updated constant name
extern const int startlimits[NUM_GATE_BOARDS][NUM_SERVOS]; // Updated constant name
extern const int endlimits[NUM_GATE_BOARDS][NUM_SERVOS]; // Updated constant name
extern const int *startlimit;
extern const int *endlimit;
extern Servo servos[NUM_SERVOS];
extern int angle[NUM_SERVOS];
extern int pastangle[NUM_SERVOS];

// Ensure extern variables are declared
unsigned long buttonPressStart[NUM_BUTTONS] = {0};
bool buttonLongPress[NUM_BUTTONS] = {false};
bool buttonShortPress[NUM_BUTTONS] = {false};
bool GateStatusLongPress[4] = {0};
bool GateStatusShortPress[4] = {0};
bool messageSent[NUM_BUTTONS] = {false}; // Add flag for message sent
bool triggerservos = false;
void blink_active() {
    static bool LED = false;
    static unsigned long internal_timer = 0;
    unsigned long time_current = millis();
    if (LED == false) {
        if (time_current > internal_timer + 500) {
            LED = true;
            digitalWrite(LED_BUILTIN, LED);
            internal_timer = time_current;
        }
    }

    if (LED == true) {
        if (time_current > internal_timer + 1500) {
            LED = false;
            digitalWrite(LED_BUILTIN, LED);
            internal_timer = time_current;
        }
    }
}

void InitializeWIFI_Serial() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    WiFi.mode(WIFI_STA);

    // Print the MAC address of the ESP32
    Serial.print("ESP32 MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Board Sel: ");
    Serial.println(BoardSel);
    Serial.print("Start Limits: ");
    for (int x = 0; x < 4; x++) {
        Serial.print(startlimit[x]);
        Serial.print(" | ");
    }
    Serial.println();
    Serial.print("End Limits: ");
    for (int x = 0; x < 4; x++) {
        Serial.print(endlimit[x]);
        Serial.print(" | ");
    }
    Serial.println();
}

void Initialize_Buttons() {
    for (int i = 0; i < ActiveButtons[BoardSel]; i++) {
        if (pull == 0) {
            pinMode(buttonPins[i], INPUT_PULLDOWN);
        } else {
            pinMode(buttonPins[i], INPUT_PULLUP);
        }
    }
}

void readButtonPresses() {
    bool buttonPressed = false;
    static bool longpressflag = false;
    for (int i = 0; i < ActiveButtons[BoardSel]; i++) {
        int buttonState = digitalRead(buttonPins[i]);
        if (buttonState == !pull && longpressflag == false) { // Check based on pull variablee
            if (buttonPressStart[i] == 0) {
                buttonPressStart[i] = millis();
            } else if (millis() - buttonPressStart[i] > 1000) { // Long press threshold
                buttonLongPress[i] = true;
                buttonShortPress[i] = false;
                buttonPressed = true;
                longpressflag = true;
            } else if (millis() - buttonPressStart[i] > 50) { // Short press threshold
                buttonShortPress[i] = true;
                buttonLongPress[i] = false;
                buttonPressed = true;
            }
        } else {
            if (buttonLongPress[i] || buttonShortPress[i]) {
                if (!messageSent[i]) {
                    if (buttonLongPress[i]) {
                        GateStatusLongPress[i] = !GateStatusLongPress[i];
                        BoardData.button = i;
                        for (int j = 0; j < NUM_BUTTONS; j++) {
                            if (j != i) {
                                GateStatusLongPress[j] = false;
                            }
                        }
                    } else if (buttonShortPress[i]) {
                        GateStatusShortPress[i] = !GateStatusShortPress[i];
                        BoardData.button = i;
                        for (int j = 0; j < NUM_BUTTONS; j++) {
                            if (j != i) {
                                GateStatusShortPress[j] = false;
                            }
                        }
                    }
                    // Ensure resetGateStatusArrays() does not reset the gate status arrays unless intended
                    resetGateStatusArrays();
                    resolvePressConflicts(GateStatusLongPress, GateStatusShortPress, NUM_BUTTONS);
                    messageSent[i] = true; // Set message sent flag
                    longpressflag = false;
                    PrintLocalGateArrays();
                    triggerservos = true;
                }
            }
            buttonPressStart[i] = 0;
            buttonLongPress[i] = false;
            buttonShortPress[i] = false;
            messageSent[i] = false; // Reset message sent flag when button is released
        }
    }
}

void resolvePressConflicts(bool longPress[], bool shortPress[], int size) {
    for (int i = 0; i < size; ++i) {
        if (longPress[i] && shortPress[i]) {
            longPress[i] = false;
        }
    }
}

void TestLED(bool shortPress[], bool longPress[], const int servoPins[]) {
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (shortPress[i] || longPress[i]) {
            digitalWrite(servoPins[i], HIGH);
            Serial.print(i);
            Serial.println(" LED ON");
        } else {
            digitalWrite(servoPins[i], LOW);
        }
    }
}

void illuminateLEDsBasedOnPress(bool shortPress[], bool longPress[], const int servoPins[]) {
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (shortPress[i] || longPress[i]) {
            digitalWrite(servoPins[i], HIGH);
        } else {
            digitalWrite(servoPins[i], LOW);
        }
    }
}


