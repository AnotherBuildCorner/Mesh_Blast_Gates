#include <ESP32Servo.h>

#define loopdelay 50
#define loopstep 1
#include <esp_now.h>
#include <WiFi.h>


#define beginlimit 0
#define endlimit 60
// Structure to receive data
typedef struct struct_message {
    bool buttonPressed;
} struct_message;

struct_message incomingData;

const int ledPin = 23;
int value = 0;
int pastVal = 0;

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
    // Copy the incoming data into the incomingData structure
    memcpy(&incomingData, data, sizeof(incomingData));
    if(incomingData.buttonPressed){
      value = endlimit;
      Serial.println(value);
    }
    else{
      value = beginlimit;
      Serial.println(value);
    }
    
}
Servo servos;  // Create a Servo object
int tempVal = 0;

void setup() {
  pinMode(32,INPUT_PULLUP);
  myServo.attach(4);  // Attach the servo to pin 25
  Serial.begin(9600);  // Start serial communication at 9600 baud
      WiFi.mode(WIFI_STA);
    delay(1000);
    Serial.print("Receiver MAC Address: ");
    Serial.println(WiFi.macAddress());
        if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onReceive);
  Serial.println("Enter a value between 0 and 180 to move the servo:");
  delay(200);
  myServo.write(beginlimit);
}

void loop() {
bool gate = digitalRead(32);
if(gate){
  value = endlimit;
}
else{
  value = beginlimit;
}

        if(pastVal > value){
          Serial.println("Past > Current");
          for(int i = pastVal; i >= value; i--){
            myServo.write(i);
            delay(loopdelay);

          }}
        
        if(pastVal < value){
          Serial.println("Past < Current");
          for(int i = pastVal; i <= value; i++){
            myServo.write(i);
            delay(loopdelay);
          }}
          pastVal = value;
}

