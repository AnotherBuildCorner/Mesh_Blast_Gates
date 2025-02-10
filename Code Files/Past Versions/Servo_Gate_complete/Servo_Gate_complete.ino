#include <ESP32Servo.h>

#define loopdelay 50
#define loopstep 1
#include <esp_now.h>
#include <WiFi.h>


#define NUM_BUTTONS 4
#define NUM_SERVOS 4
#define NUM_PEERS 3
#define waittime 1000

const int buttonPins[NUM_BUTTONS] = {0, 1, 2, 21};  // Change as per your setup
const int servoPins[NUM_SERVOS] = {23, 22, 18, 20};        // Change as per your setup
const int startlimit[NUM_SERVOS] = {0,0,0,0};
const int endlimit[NUM_SERVOS]={60,60,60,60};
int angle[NUM_SERVOS] = {0,0,0,0};
int pastangle[NUM_SERVOS] = {0,0,0,0};
bool gateflag = true;

Servo servos[NUM_SERVOS]; 

typedef struct struct_message {
  int buttonIndex;
} struct_message;

struct_message myData;

const uint8_t broadcastAddresses[NUM_PEERS][6] = {
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF4, 0xAC}, // 2pin
  {0xF0, 0xF5, 0xBD, 0x2C, 0xEF, 0xCC}, //4 pin a
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF4, 0x54}, // 4 pin b
  //{0x24, 0x6F, 0x28, 0xAE, 0xDC, 0x52},
  //{0x24, 0x6F, 0x28, 0xAE, 0xDC, 0x53},
  //{0x24, 0x6F, 0x28, 0xAE, 0xDC, 0x54}
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  
  WiFi.mode(WIFI_STA);

  // Print the MAC address of the ESP32
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);
    angle[i] = startlimit[i];
    pastangle[i] = angle[i];
    servos[i].write(angle[i]);
    delay(200);
  }
  pastangle[0]=endlimit[0];
  angle[0] = endlimit[0];
  servos[0].write(angle[0]);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  for (int i = 0; i < NUM_PEERS; i++) {
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddresses[i], 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }

}



void loop() {
  digitalWrite(LED_BUILTIN,LOW);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == HIGH) {
      myData.buttonIndex = i;

      lightLED(i);
      Serial.println(servoPins[i]);
      delay(waittime);  // Adjust delay as needed
      for (int j = 0; j < NUM_PEERS; j++) {
        esp_now_send(broadcastAddresses[j], (uint8_t *) &myData, sizeof(myData));
      }
      turnOffAllLEDs(i);
      gateflag = false;
    }
  }

for(int j = 0; j < NUM_SERVOS; j++){


        
        if(pastangle[j] < angle[j]){
          Serial.print("Gate ");
          Serial.print(j);
          Serial.println(" Opening");
          gateflag = true;
          for(int i = pastangle[j]; i <= angle[j]; i++){
            //Serial.println(i);
            servos[j].write(i);
            delay(loopdelay);
          }pastangle[j] = angle[j];}
          
                  
        if(pastangle[j] > angle[j] && gateflag == true){
          Serial.print("Gate ");
          Serial.print(j);
          Serial.println(" Closing");
          for(int i = pastangle[j]; i >= angle[j]; i--){
            //Serial.println(i);
            servos[j].write(i);
            delay(loopdelay);

          }
          pastangle[j] = angle[j];}
}

}


void lightLED(int index) {
  angle[index] = endlimit[index];
  Serial.print(index);
  Serial.println(servoPins[index]);

  //digitalWrite(ledPins[index], HIGH);
}

void turnOffAllLEDs(int index) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    if(i != index){
      angle[i] = startlimit[i];

      //digitalWrite(ledPins[i], LOW);
      }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Button index received: ");
  Serial.println(myData.buttonIndex);
  delay(waittime);  // Adjust delay as needed
  turnOffAllLEDs(5);
}