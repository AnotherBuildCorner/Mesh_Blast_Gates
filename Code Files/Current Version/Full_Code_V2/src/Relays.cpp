#include "Relays.h"
#include "Mesh_Network.h"

// Define and initialize the variables
const int relayPins[NUM_RELAYS] = {22, 23}; // Example pin numbers, change as needed
bool relayStates[NUM_RELAYS] = {false, false};
const int debounceTime = 200;

void Initialize_Relays() {
    for (int i = 0; i < NUM_RELAYS; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW); // Initialize relays to off state
    }
}
void Initialize_Relay_Buttons() {
    pinMode(PUSHBUTTON1_PIN, INPUT_PULLUP);
    pinMode(PUSHBUTTON2_PIN, INPUT_PULLUP);
}

void SetRelayState(int relayIndex, bool state) {
    if (relayIndex >= 0 && relayIndex < NUM_RELAYS) {
        relayStates[relayIndex] = state;
        digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
    }
}

void ToggleRelay(int relayIndex) {
    if (relayIndex >= 0 && relayIndex < NUM_RELAYS) {
        relayStates[relayIndex] = !relayStates[relayIndex];
        digitalWrite(relayPins[relayIndex], relayStates[relayIndex] ? HIGH : LOW);
    }
}

void ControlRelayWithCollectorState(bool state) {
    static unsigned long lastDebounceTime = 0;
    if (millis() - lastDebounceTime > debounceTime) {
        SetRelayState(0, state);
        lastDebounceTime = millis();
    }
}

void ControlRelayWithExtraState(bool state) {
    static unsigned long lastDebounceTime = 0;
    if (millis() - lastDebounceTime > debounceTime) {
        SetRelayState(1, state);
        lastDebounceTime = millis();
    }
}

void ReadPushbuttonsAndControlStates() {
    static bool lastButton1State = LOW;
    static bool lastButton2State = LOW;
    bool button1State = digitalRead(PUSHBUTTON1_PIN);
    bool button2State = digitalRead(PUSHBUTTON2_PIN);

    if (button1State != lastButton1State) {
        if (button1State == HIGH && lastButton1State == LOW) {
            BoardData.CollectorState = !BoardData.CollectorState;
            ToggleRelay(0);
            esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
        }
        lastButton1State = button1State;
    }

    if (button2State != lastButton2State) {
        if (button2State == HIGH && lastButton2State == LOW) {
            BoardData.ExtraState = !BoardData.ExtraState;
            ToggleRelay(1);
            esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
        }
        lastButton2State = button2State;
    }
}
