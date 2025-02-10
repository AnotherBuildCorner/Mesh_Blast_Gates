#include <WiFi.h>
#include <esp_now.h>

#define NUM_TOUCHPADS 9
#define TOUCH_THRESHOLD 150000 // Adjust threshold as needed
#define SHUTDOWN_SIGNAL false // Signal to turn off the relay
//#define SLEEP_TIME 1200000000 // 20 minutes in microseconds (20 * 60 * 1000 * 1000)
#define Sleep_Min 1
#define LED_PIN 44
#define PWR_LED 43

#define flashtime 200
#define flashcount 5

const int touchPins[NUM_TOUCHPADS] = {1,2, 3, 4, 5, 6,7,8,9}; // Touch sensor pins
//const int ledPins[NUM_TOUCHPADS] = {32, 33, 25, 23, 19, 21}; // LED pins

bool relayStates[NUM_TOUCHPADS] = {false, false, false, false, false, false,false,false,false};
//bool lastSentSuccess[NUM_TOUCHPADS] = {true, true, true, true, true, true}; // Track last send status
bool lastSentSuccess= true;
bool ContactorState = false;
//uint8_t receiverMac[] = {0x40, 0x22, 0xD8, 0x77, 0x15, 0xA4}; // Change to your receiver's MAC address
uint8_t receiverMac[] = {0xDC, 0xDA, 0x0C, 0xCE, 0xBD, 0xCC}; // Change to your receiver's MAC address
unsigned long startTime;
const unsigned long SLEEP_TIME = Sleep_Min*60*1000;
int touchPin = 0;
int touchValue = 0;

void setup() {
    Serial.begin(115200);
    delay(20); // Give some time for the serial monitor to connect
    touchPin = esp_sleep_get_touchpad_wakeup_status();
    Serial.println(touchPin);
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Starting setup...");
    //pinMode(LED_PIN, OUTPUT);
    //digitalWrite(LED_PIN, LOW);
    //pinMode(PWR_LED, OUTPUT);
    //digitalWrite(PWR_LED, HIGH);
    for (int i = 0; i < NUM_TOUCHPADS; i++) {
        touchAttachInterrupt(touchPins[i], NULL, TOUCH_THRESHOLD);
    }
    // Initialize LEDs
    //for (int i = 0; i < NUM_TOUCHPADS; i++) {
        //pinMode(ledPins[i], OUTPUT);
        //digitalWrite(ledPins[i], LOW);
        //Serial.print("Initialized LED pin ");
       // Serial.println(ledPins[i]);
    //}

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
    //esp_now_register_recv_cb(onDataReceived);

    // Initialize the start time
    startTime = millis();
    Serial.println("Setup complete");
}

void loop() {
    for (int i = 0; i < NUM_TOUCHPADS; i++) {
        int prev = touchValue;
        touchValue = touchRead(touchPins[i]);

        if (touchValue > TOUCH_THRESHOLD) {
            delay(50); // Debounce delay
            while (touchRead(touchPins[i]) > TOUCH_THRESHOLD); // Wait for touch release

            Serial.print("Touchpad ");
            Serial.print(i);
            Serial.print(" activated");
            Serial.print("  Touch Value: ");
            Serial.print(touchValue);
            Serial.print(" Pre-Touch: ");
            Serial.println(prev);

            // Deactivate all other touchpads
            for (int j = 0; j < NUM_TOUCHPADS; j++) {
                if (j != i && relayStates[j]) {
                    relayStates[j] = false;
                    //esp_now_send(receiverMac, (uint8_t *)&relayStates[j], sizeof(relayStates[j]));
                   // digitalWrite(ledPins[j], LOW);
                    Serial.print("Deactivated touchpad ");
                    Serial.println(j);
                }
            }

            relayStates[i] = !relayStates[i]; // Toggle the current touchpad's state

            bool anyRelayOn = false;
            for (int i = 0; i < NUM_TOUCHPADS; i++) {
            if (relayStates[i]) {
                anyRelayOn = true;
                break; // No need to check further if any relay is ON
            }
        }

            esp_err_t result = esp_now_send(receiverMac, (uint8_t *)relayStates, sizeof(relayStates)); //Looks like it only sends one state at a time. this needs changed for the future

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

            //digitalWrite(ledPins[i], lastSentSuccess[i] && relayStates[i] ? HIGH : LOW);
            startTime = millis(); // Reset the start time
        }
    }

    // Check if 20 minutes have passed
    if (millis() - startTime >= SLEEP_TIME) {
        Serial.println("Entering deep sleep...");
        //sendShutdownSignal();
        enterDeepSleep();
    }
}

void sendShutdownSignal() {
    for(int i = 0; i < NUM_TOUCHPADS; i++){
      relayStates[i] = false;
    }
    esp_now_send(receiverMac, (uint8_t *)relayStates, sizeof(relayStates));
    delay(100); // Give time for the signal to be sent
    Serial.println("Shutdown signal sent");
}

void enterDeepSleep() {
    esp_sleep_enable_touchpad_wakeup();
    esp_deep_sleep_start();
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
    // Ensure data received is for relay control and valid
    if (len == sizeof(bool)) {
        bool receivedData;
        memcpy(&receivedData, data, sizeof(receivedData)); // Copy received data to receivedData variable

        // Update the contactor state based on received data
        ContactorState = receivedData;

        // Print received contactor state for debugging
        Serial.print("Received contactor state: ");
        Serial.println(ContactorState ? "ON" : "OFF");

        // Optionally, update LED feedback based on contactor state
        digitalWrite(LED_PIN, ContactorState ? HIGH : LOW);
    } else {
        Serial.println("Invalid data received or incorrect length");
        Serial.print("Expected length: ");
        Serial.println(sizeof(bool));
        Serial.print("Received length: ");
        Serial.println(len);
    }
}


void errorFlash(){
  bool LED = true;
  for(int i = 0; i < flashcount*2; i++){
      digitalWrite(LED_PIN,LED);
      LED = !LED;
      delay(flashtime);
}}
