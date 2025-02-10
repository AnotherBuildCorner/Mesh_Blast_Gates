#include <WiFi.h>
#include <esp_now.h>

#define NUM_RELAYS 9
#define WAKE_THRESHOLD 200000 // Adjust threshold as needed
#define SLEEP_MIN 1
#define LED_PIN 3
#define PWR_LED 43
#define WAKEUP_BUTTON_PIN 2 // GPIO pin for the wake-up button

#define FLASHTIME 200
#define FLASHCOUNT 5

bool relayStates[NUM_RELAYS] = {false};
bool lastSentSuccess = true;
bool ContactorState = false;
uint8_t receiverMac[] = {0xDC, 0xDA, 0x0C, 0xCE, 0xBD, 0xCC}; // Change to your receiver's MAC address
unsigned long startTime;
const unsigned long SLEEP_TIME = SLEEP_MIN * 60 * 1000;
bool anyRelayOn = false;
void setup() {
    Serial.begin(115200);
    delay(20); // Give some time for the serial monitor to connect

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Starting setup...");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 1);
    pinMode(PWR_LED, OUTPUT);
    digitalWrite(PWR_LED, HIGH);

    // Configure push button pin for wake-up
    pinMode(WAKEUP_BUTTON_PIN, INPUT_PULLDOWN);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_BUTTON_PIN, HIGH);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("WiFi disconnected");

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    Serial.println("ESP-NOW initialized");

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
    Serial.println("Peer added");

    startTime = millis();
    Serial.println("Setup complete");
}

void loop() {

    if (digitalRead(WAKEUP_BUTTON_PIN) == HIGH) {
        delay(50); // Debounce delay

        while (digitalRead(WAKEUP_BUTTON_PIN) == HIGH); // Wait for button release
        delay(50);

        Serial.println("Wake-up button activated");

        // Toggle relay state
        relayStates[0] = !relayStates[0];

        bool anyRelayOn = false;
        for (int i = 0; i < NUM_RELAYS; i++) {
            if (relayStates[i]) {
                anyRelayOn = true;
                break;
            }
        }

        esp_err_t result = esp_now_send(receiverMac, (uint8_t *)relayStates, sizeof(relayStates));

        if (result == ESP_OK) {
            
            Serial.println("Wake-up button: Sent with success");
            lastSentSuccess = true;
            digitalWrite(LED_PIN, anyRelayOn);
        } else {
            Serial.println("Wake-up button: Error sending the data");
            lastSentSuccess = false;
            errorFlash();
        }

        startTime = millis(); // Reset the start time
    }

    if (millis() - startTime >= SLEEP_TIME) {
        Serial.println("Entering deep sleep...");
        enterDeepSleep();
    }
}

void enterDeepSleep() {
    Serial.println("Preparing to sleep...");
    // Optional: Disable WiFi to save more power before sleep
    WiFi.disconnect(true);
    delay(100); // Give time for WiFi to disconnect properly

    esp_deep_sleep_start();
}

void errorFlash() {
    bool LED = true;
    for (int i = 0; i < FLASHCOUNT * 2; i++) {
        digitalWrite(LED_PIN, LED);
        LED = !LED;
        delay(FLASHTIME);
    }
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



