#include "Relays.h"
#include "Mesh_Network.h"

// Define and initialize the variables
const int relayPins[NUM_RELAYS] = {22, 23}; // Example pin numbers, change as needed
bool relayStates[NUM_RELAYS] = {false, false}; // Ensure initial states are LOW
const int debounceTime = 200;
const bool startstate = LOW;
void Initialize_Relays() {
    for (int i = 0; i < NUM_RELAYS; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW); // Initialize relays to off state
    }
    BoardData.CollectorState = LOW; // Ensure CollectorState is LOW on startup
    BoardData.ExtraState = LOW; // Ensure ExtraState is LOW on startup
    Serial.println("CollectorState initialized to LOW");
    Serial.println("ExtraState initialized to LOW");
}

void Initialize_Relay_Buttons() {
    pinMode(PUSHBUTTON1_PIN, INPUT_PULLUP);
    pinMode(PUSHBUTTON2_PIN, INPUT_PULLUP);
}

void SetRelayState(int relayIndex, bool state) {
    if (relayIndex >= 0 && relayIndex < NUM_RELAYS) {
        relayStates[relayIndex] = state;
        digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
        Serial.println("Relay " + String(relayIndex) + " state set to " + String(state));
    }
}

void ToggleRelay(int relayIndex) {
    if (relayIndex >= 0 && relayIndex < NUM_RELAYS) {
        relayStates[relayIndex] = !relayStates[relayIndex];
        digitalWrite(relayPins[relayIndex], relayStates[relayIndex] ? HIGH : LOW);
        Serial.println("Relay " + String(relayIndex) + " toggled to " + String(relayStates[relayIndex]));
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

void ControlFromIncoming() {
    if (BoardData.CollectorState != relayStates[0]) {
        SetRelayState(0, BoardData.CollectorState);
        Serial.println("CollectorState changed to " + String(BoardData.CollectorState));
    }
    if (BoardData.ExtraState != relayStates[1]) {
        SetRelayState(1, BoardData.ExtraState);
        Serial.println("ExtraState changed to " + String(BoardData.ExtraState));
    }
}

void ReadPushbuttonsAndControlStates() {
    static bool lastButton1State = startstate;
    static bool lastButton2State = startstate;
    bool button1State = digitalRead(PUSHBUTTON1_PIN);
    bool button2State = digitalRead(PUSHBUTTON2_PIN);

    if (button1State != lastButton1State) {
        if (button1State == startstate && lastButton1State == !startstate) {
            BoardData.CollectorState = !BoardData.CollectorState;
            ToggleRelay(0);
            esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
            Serial.println("Button 1 pressed, CollectorState toggled to " + String(BoardData.CollectorState));
        }
        lastButton1State = button1State;
    }

    if (button2State != lastButton2State) {
        if (button2State == startstate && lastButton2State == !startstate) {
            BoardData.ExtraState = !BoardData.ExtraState;
            ToggleRelay(1);
            esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
            Serial.println("Button 2 pressed, ExtraState toggled to " + String(BoardData.ExtraState));
        }
        lastButton2State = button2State;
    }
}
