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
#define NUM_PEERS 4

typedef struct struct_message {
  bool CollectorState;
  bool ExtraState;
  int button;
  int board = 0;
  bool reboot;
} struct_message;

struct_message myData;

const uint8_t broadcastAddresses[NUM_PEERS][6] = {
  {0xF0, 0xF5, 0xBD, 0x2D, 0x14, 0x4C}, // #1  Bandsaw board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF4, 0x8C}, // #2  Chopsaw Board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xFA, 0xC8}, // #3  Tablesaw Board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF9, 0xE0}, // DC V2
  // Unused addresses commented out
};

bool relayStates[NUM_RELAYS] = {false};
bool lastSentSuccess = true;
bool ContactorState = false;
unsigned long startTime;
const unsigned long SLEEP_TIME = SLEEP_MIN * 60 * 1000;
bool anyRelayOn = false;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
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

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized");

  esp_now_register_send_cb(OnDataSent);

  for (int i = 0; i < NUM_PEERS; i++) {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo)); // Initialize the struct
    memcpy(peerInfo.peer_addr, broadcastAddresses[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }
  
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
    myData.CollectorState = ! myData.CollectorState;

    for (int j = 0; j < NUM_PEERS; j++) {
      esp_now_send(broadcastAddresses[j], (uint8_t *) &myData, sizeof(myData));
      startTime = millis(); // Reset the start time
    }
  }

  if (millis() - startTime >= SLEEP_TIME) {
    Serial.println("Entering deep sleep...");
    enterDeepSleep();
  }

  if(myData.CollectorState == false){
    analogWrite(LED_PIN,20);
  } else {
    analogWrite(LED_PIN,255);
  }
}

void enterDeepSleep() {
  Serial.println("Preparing to sleep...");
  // Optional: Disable WiFi to save more power before sleep
  WiFi.disconnect(true);
  delay(100); // Give time for WiFi to disconnect properly
  esp_deep_sleep_start();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


