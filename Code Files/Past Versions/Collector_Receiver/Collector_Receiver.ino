#include <WiFi.h>
#include <esp_now.h>

#define NUM_RELAYS 9 // Assuming 6 relays in total
#define DEBOUNCE_DELAY 50 // Debounce delay in milliseconds
#define ContactorPin 19
#define Sleep_Min 60

bool relayStates[NUM_RELAYS] = {false, false, false, false, false, false,false,false,false}; // Initial relay states
bool lastRelayStates[NUM_RELAYS] = {false, false, false, false, false, false,false,false,false}; // Last known relay states
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


void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
    // Ensure data received is for relay control and valid
    recflag = true;
    if (len == sizeof(relayStates)) {
        memcpy(relayStates, data, sizeof(relayStates)); // Copy received relay states to relayStates array

        // Print received relay states for debugging
        Serial.print("Received relay states: ");
        for (int i = 0; i < NUM_RELAYS; i++) {
            Serial.print(relayStates[i] ? "1" : "0");
        }
        Serial.println();

        // Update relays based on received states
        for (int i = 0; i < NUM_RELAYS; i++) {
            // Check if the received state is different from the last known state
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
                break; // No need to check further if any relay is ON
            }
        }

        
        //esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&anyRelayOn, sizeof(bool));
        //Serial.println(result);
        digitalWrite(ContactorPin, anyRelayOn ? HIGH : LOW);
    } else {
        Serial.println("Invalid data received or incorrect length");
        Serial.print("Expected length: ");
        Serial.println(sizeof(relayStates));
        Serial.print("Received length: ");
        Serial.println(len);
    }
    startTime = millis();
}

