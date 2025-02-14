#include <WiFi.h>
#include <esp_now.h>

#define NUM_RELAYS 2 // Assuming 6 relays in total
#define DEBOUNCE_DELAY 50 // Debounce delay in milliseconds
#define ContactorPin 22
#define ExtraPin 23
#define Sleep_Min 120
#define NUM_BUTTONS 2

bool CollectorStates[2] = {false,false};
bool ExtraStates[2] = {false,false};

const int buttonPins[2] = {2, 21};  // Change as per your setup
const bool pull = 0; //0 for down 1 for up

unsigned long lastChangeTime = {0}; // Time of last relay state change
bool recflag = false;
uint8_t receiverMac[] = {0xCC, 0xDB, 0xA7, 0x1E, 0xF7, 0xF8};

unsigned long startTime;
const unsigned long SLEEP_TIME = Sleep_Min*60*1000;


bool quesend[NUM_BUTTONS];
unsigned long timer0[NUM_BUTTONS],timer1[NUM_BUTTONS];
unsigned long t2;
bool timerlock[NUM_BUTTONS];

typedef struct struct_message {
  bool CollectorState;
  bool ExtraState;
  int button;
  int board;
  bool reboot;
} struct_message;

struct_message myData;


void setup() {
    Serial.begin(115200);
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    pinMode(ContactorPin, OUTPUT);
    pinMode(LED_BUILTIN,OUTPUT);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("Receiver setup complete");
    startTime = millis();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    if(pull == 0){
    pinMode(buttonPins[i], INPUT_PULLDOWN);}
    else{
      pinMode(buttonPins[i], INPUT_PULLUP);
    }
  }
}

void loop() {
  blink_active();
    if (millis() - startTime >= SLEEP_TIME && recflag == true) {
      Serial.println("Timer Sleep Activated");
      digitalWrite(ContactorPin, false);
      recflag = false;
    }

  for (int i = 0; i < NUM_BUTTONS; i++) {
      if (digitalRead(buttonPins[i]) == pull){  // reset timer flag
      
        timerlock[i] = false;
        timer0[i] = millis();
      }

    if (digitalRead(buttonPins[i]) == !pull) { // set the initial case
      if(timerlock[i] == false){
      timer0[i] = millis();
      timerlock[i] = true;
      }
      if(millis()-timer0[i] >= 100){
        quesend[i] = true;}}

      if(quesend[i] == true && digitalRead(buttonPins[i]) == pull){
        quesend[i] = false;       
        t2 = millis();
        Serial.print("Button: ");
        Serial.print(i+1);
        Serial.print("Pressed.  Timers: ");
        Serial.print(timer0[i]);
        Serial.print("  ");
        Serial.print(t2);
        timer1[i] = timer0[i];
        timer0[i] - millis();
        timerlock[i] = false;

        if(i == 0){
          CollectorStates[0] = !CollectorStates[0];
          digitalWrite(ContactorPin, CollectorStates[0]);
        }
        if(i == 1){
          ExtraStates[0] = !ExtraStates[0];
          digitalWrite(ExtraPin, ExtraStates[0]);
        }

}}}


void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
    recflag = true;

  memcpy(&myData, incomingData, sizeof(myData));
  CollectorStates[0]=myData.CollectorState;
  ExtraStates[0] = myData.ExtraState;

  if(myData.reboot == true){
    Serial.println("Received reboot command");
    esp_restart();
  }
        if (CollectorStates[1] != CollectorStates[0]) {
            unsigned long currentTime = millis();
            // Apply debounce delay to ensure stable state change
            if (currentTime - lastChangeTime >= DEBOUNCE_DELAY) {
                // Update relay state and print the change
                CollectorStates[1] = CollectorStates[0];
                Serial.print("Collector ");
                Serial.println(CollectorStates[0] ? " ON" : " OFF");
                lastChangeTime = currentTime;
                digitalWrite(ContactorPin, CollectorStates[0]);
            }
        }
    
       
       if (ExtraStates[1] != ExtraStates[0]) {
            unsigned long currentTime = millis();
            // Apply debounce delay to ensure stable state change
            if (currentTime - lastChangeTime >= DEBOUNCE_DELAY) {
                // Update relay state and print the change
                ExtraStates[1] = ExtraStates[0];
                Serial.print("Extra ");
                Serial.println(ExtraStates[0] ? " ON" : " OFF");
                lastChangeTime = currentTime;
                digitalWrite(ExtraPin, ExtraStates[0]);
            }
        }

    

    startTime = millis();
}

void blink_active(){
  static bool LED = false;
  static unsigned long internal_timer = 0;
  unsigned long time_current = millis() ;
  if(LED == false){
    if(time_current > internal_timer+500){
      LED = true;
      digitalWrite(LED_BUILTIN,LED);
      internal_timer = time_current;
    }}

  if(LED == true){
    if(time_current > internal_timer+1500){
      LED = false;
      digitalWrite(LED_BUILTIN,LED);
      internal_timer = time_current;
    }}

}