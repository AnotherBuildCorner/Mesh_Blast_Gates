#include <WiFi.h>
#include <esp_now.h>


#define NUM_TOUCHPADS 1
#define NUM_RELAYS 9
#define TOUCH_THRESHOLD 120000
#define WAKE_THRESHOLD 800000 // Adjust threshold as needed
#define SHUTDOWN_SIGNAL false // Signal to turn off the relay
#define SLEEP_MIN 10
#define LED_PIN 44
#define PWR_LED 43
#define WAKEUP_BUTTON_PIN 9
#define FLASHTIME 200
#define FLASHCOUNT 5

const int touchPins[NUM_TOUCHPADS] = {1}; // Touch sensor pins

bool relayStates[NUM_RELAYS] = {false, false, false, false, false, false, false, false, false};
bool lastSentSuccess = true;
bool ContactorState = false;
uint8_t receiverMac[] = {0xDC, 0xDA, 0x0C, 0xCE, 0xBD, 0xCC}; // Change to your receiver's MAC address
unsigned long startTime;
const unsigned long SLEEP_TIME = SLEEP_MIN * 60 * 1000;
int touchPin = 0;
int touchValue = 0;
int prev[NUM_TOUCHPADS] = {0};

void setup() {
    Serial.begin(115200);
    delay(20); // Give some time for the serial monitor to connect

    // Add a small delay before checking the touchpad wakeup status
    delay(100); // 100ms delay
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD) {
        Serial.println("Woke up from touchpad");
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Woke up from external push button");
    } else {
        Serial.println("Woke up from other cause");
    }
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Starting setup...");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(PWR_LED, OUTPUT);
    digitalWrite(PWR_LED, HIGH);

    // Configure push button pin for wake-up
    pinMode(WAKEUP_BUTTON_PIN, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_BUTTON_PIN, LOW);

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
    for (int i = 0; i < NUM_TOUCHPADS; i++) {
        prev[i] = touchValue;
        touchValue = touchRead(touchPins[i]);

        if (touchValue > TOUCH_THRESHOLD) {
            delay(50); // Debounce delay
            // Check the touch value multiple times to confirm it's a valid touch
            bool validTouch = true;
            for (int j = 0; j < 5; j++) {
                if (touchRead(touchPins[i]) <= TOUCH_THRESHOLD) {
                    validTouch = false;
                    break;
                }
                delay(10); // Additional debounce delay
            }

            if (!validTouch) {
                continue; // Skip this iteration if not a valid touch
            }

            while (touchRead(touchPins[i]) > TOUCH_THRESHOLD); // Wait for touch release

            Serial.print("Touchpad ");
            Serial.print(i);
            Serial.print(" activated");
            Serial.print("  Touch Value: ");
            Serial.print(touchValue);
            Serial.print(" Pre-Touch: ");
            Serial.println(prev[i]);

            for (int j = 0; j < NUM_TOUCHPADS; j++) {
                if (j != i && relayStates[j]) {
                    relayStates[j] = false;
                    Serial.print("Deactivated touchpad ");
                    Serial.println(j);
                }
            }

            relayStates[i] = !relayStates[i]; // Toggle the current touchpad's state

            bool anyRelayOn = false;
            for (int i = 0; i < NUM_TOUCHPADS; i++) {
                if (relayStates[i]) {
                    anyRelayOn = true;
                    break;
                }
            }

            esp_err_t result = esp_now_send(receiverMac, (uint8_t *)relayStates, sizeof(relayStates));

            if (result == ESP_OK) {
                digitalWrite(LED_PIN, anyRelayOn);
                Serial.print("Touchpad ");
                Serial.print(i);
                Serial.println(": Sent with success");
                lastSentSuccess = true;
            } else {
                Serial.print("Touchpad ");
                Serial.print(i);
                Serial.println(": Error sending the data");
                lastSentSuccess = false;
                errorFlash();
            }

            startTime = millis(); // Reset the start time
        }
    }

    //if (millis() - startTime >= SLEEP_TIME) {
     //   enterDeepSleep();}
}

void enterDeepSleep() {
    Serial.println("Preparing to sleep...");
    // Optional: Disable WiFi to save more power before sleep
    WiFi.disconnect(true);
    delay(100); // Give time for WiFi to disconnect properly

    // Enter deep sleep
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



