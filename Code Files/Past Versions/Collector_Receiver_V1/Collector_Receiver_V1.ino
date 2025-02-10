#include <WiFi.h>
#include <esp_now.h>

#define NUM_RELAYS 1 // Assuming 6 relays in total
#define DEBOUNCE_DELAY 50 // Debounce delay in milliseconds
#define ContactorPin 19
#define Sleep_Min 60

bool relayStates[NUM_RELAYS] = {false}; // Initial relay states
bool lastRelayStates[NUM_RELAYS] = {false}; // Last known relay states

unsigned long lastChangeTime = {0}; // Time of last relay state change
bool ContactorState = false;
bool recflag = false;
uint8_t receiverMac[] = {0xCC, 0xDB, 0xA7, 0x1E, 0xF7, 0xF8};

unsigned long startTime;
const unsigned long SLEEP_TIME = Sleep_Min*60*1000;

void setup() {
    Serial.begin(115200);
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    pinMode(ContactorPin, OUTPUT);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onDataReceived);
    Serial.println("Receiver setup complete");
    startTime = millis();
}

void loop() {
    if (millis() - startTime >= SLEEP_TIME && recflag == true) {
      Serial.println("Timer Sleep Activated");
      digitalWrite(ContactorPin, false);
      recflag = false;
    }
}


void onDataReceived(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    recflag = true;

    // Ensure data received is for relay control and valid
    memcpy(relayStates, data, sizeof(relayStates)); // Copy received relay states to relayStates array

    for (int i = 0; i < NUM_RELAYS; i++) {
        if (relayStates[i] != lastRelayStates[i]) {
            unsigned long currentTime = millis();

            // Apply debounce delay to ensure stable state change
            if (currentTime - lastChangeTime >= DEBOUNCE_DELAY) {
                // Update relay state and print the change
                lastRelayStates[i] = relayStates[i];
                Serial.print("Relay ");
                Serial.print(i);
                Serial.println(relayStates[i] ? " ON" : " OFF");
                lastChangeTime = currentTime;
            }
        }
    }

    bool anyRelayOn = false;
    for (int i = 0; i < NUM_RELAYS; i++) {
        if (relayStates[i]) {
            anyRelayOn = true;
            break;
        }
    }

    digitalWrite(ContactorPin, anyRelayOn ? HIGH : LOW);
    startTime = millis();
}

