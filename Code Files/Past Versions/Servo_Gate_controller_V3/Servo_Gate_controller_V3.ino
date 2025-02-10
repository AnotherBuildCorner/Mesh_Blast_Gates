/* V3 has changed pinouts, and hopefully will add power monitoring controls to stop burning chipsets.

*/

#include <ESP32Servo.h>

#define loopdelay 30
#define loopstep 1
#include <esp_now.h>
#include <WiFi.h>
const bool pull = 0; //0 for down 1 for up
#define debounce 100
#define NUM_BUTTONS 4
#define NUM_SERVOS 4
#define NUM_PEERS 4
#define NUM_BOARDS 3
#define waittime 1000
#define BoardSel 3

#define vss 5 // 6.1
#define vcc 3.3
#define vratio 2
#define Rsense 0.47
#define ADCmax 3000.0
#define s_or_d 1
#define currentreadout 0
#define currentThresh 1.1
const int powerPins[s_or_d] = {0,};
float powerread[s_or_d];
float voltages[s_or_d];
float current;
const int buttonPins[NUM_BUTTONS] = {1, 2, 21, 22};  // Change as per your setup
const int servoPins[NUM_SERVOS] = {23, 19, 20, 18};        // Change as per your setup
const int ActiveButtons[NUM_BOARDS] = {2,4,2};
//const int startlimit[NUM_SERVOS] = {0,0,0,0};  // Bandsaw, Spare Port
//const int endlimit[NUM_SERVOS]={150,160,60,60};
//const int startlimit[NUM_SERVOS] = {0,0,0,0}; // Planer, Chop L, Chop R, Router
//const int endlimit[NUM_SERVOS]={140,140,140,150};
//const int startlimit[NUM_SERVOS] = {0,0,0,0};  // Tablesaw,  side port
//const int endlimit[NUM_SERVOS]={145,150,60,60};

const int startlimits[NUM_BOARDS][NUM_SERVOS] = {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
  };  // Tablesaw,  side port
const int endlimits[NUM_BOARDS][NUM_SERVOS]={
{150,160,60,60}, //Bandsaw
{140,140,140,150}, //Chop Saw
{145,150,60,60}, // Table Saw

};

const int* startlimit = startlimits[BoardSel-1];
const int* endlimit = endlimits[BoardSel-1];

unsigned long timer0[NUM_BUTTONS],timer1[NUM_BUTTONS];
bool timerlock[NUM_BUTTONS];

int angle[NUM_SERVOS] = {0,0,0,0};
int pastangle[NUM_SERVOS] = {0,0,0,0};
bool gateflag = true;
int last_active = 10;

Servo servos[NUM_SERVOS]; 

typedef struct struct_message {
  bool buttonIndex;
  int button;
  int board;
} struct_message;

struct_message myData;

const uint8_t broadcastAddresses[NUM_PEERS][6] = {
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF9, 0x40}, // #1  Bandsaw board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF4, 0x8C}, //#2   Chopsaw Board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xFA, 0xC8}, // #3  Tablesaw Board
  {0xDC, 0xDA, 0x0C, 0xCE, 0xBD, 0xCC}, // Dust collector  last is CC  not CD
//{0xDC, 0xDA, 0x0C, 0xCE, 0xF4, 0x8C},  //4 pin
  //{0x24, 0x6F, 0x28, 0xAE, 0xDC, 0x54}
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  
  WiFi.mode(WIFI_STA);



  // Print the MAC address of the ESP32
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Board Sel: ");
  Serial.println(BoardSel);
  myData.board = BoardSel;
  Serial.print("Start Limits: ");
  for(int x = 0; x < 4; x++){
  Serial.print(startlimit[x]);
  Serial.print(" | ");}
  Serial.println();
  Serial.print("End Limits: ");
  for(int x = 0; x < 4; x++){
  Serial.print(endlimit[x]);
  Serial.print(" | ");}
  Serial.println();
  

  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);
    angle[i] = startlimit[i];
    pastangle[i] = angle[i];
    servos[i].write(angle[i]);
    delay(1000);
  }


  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].detach();}

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

  for (int i = 0; i < ActiveButtons[BoardSel-1]; i++) {
    if(pull == 0){
    pinMode(buttonPins[i], INPUT_PULLDOWN);}
    else{
      pinMode(buttonPins[i], INPUT_PULLUP);
    }
  }

for (int i = 0; i < s_or_d; i++){
  pinMode(powerPins[i],INPUT);
}
Serial.println("Startup Complete");
}



void loop() {
  digitalWrite(LED_BUILTIN,LOW);
  readCurrent();


  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == !pull) {
      if(timerlock[i] == false){
      timer0[i] = millis();
      timerlock[i] = true;
      }

      if (digitalRead(buttonPins[i]) == pull){  // reset timer flag
        timerlock[i] = false;
      }
    
      
      if(timer0[i] - millis() >= debounce && digitalRead(buttonPins[i]) == !pull){
        Serial.print("Button: ");
        Serial.print(i+1);
        Serial.print("Pressed.  Timers: ");
        Serial.print(timer0[i]);
        Serial.print("  ");
        Serial.print(millis());
        timer1[i] = timer0[i];
        myData.button = i+1;
             
      if(last_active == i){
        myData.buttonIndex = !myData.buttonIndex;
      }
      else{
        myData.buttonIndex = 1;
      }
      last_active = i;

      lightLED(i);
      delay(waittime);  // Adjust delay as needed
      for (int j = 0; j < NUM_PEERS; j++) {
        esp_now_send(broadcastAddresses[j], (uint8_t *) &myData, sizeof(myData));
      }
      turnOffAllLEDs(i);
      gateflag = false;
      }
    }
  }

for(int j = 0; j < NUM_SERVOS; j++){
        
        if(pastangle[j] < angle[j]){
          servos[j].attach(servoPins[j]);
          Serial.print("Gate ");
          Serial.print(j+1);
          Serial.println(" Opening");
          gateflag = true;
          for(int i = pastangle[j]; i <= angle[j]; i++){
            //Serial.println(i);
            servos[j].write(i);
            delay(loopdelay);
          }
          servos[j].detach();
          pastangle[j] = angle[j];}
          
                  
        if(pastangle[j] > angle[j] && gateflag == true){
          servos[j].attach(servoPins[j]);
          Serial.print("Gate ");
          Serial.print(j);
          Serial.println(" Closing");
          for(int i = pastangle[j]; i >= angle[j]; i--){
            //Serial.println(i);
            servos[j].write(i);
            delay(loopdelay);

          }
          servos[j].detach();
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
  last_active = 10;
  Serial.print("DC State");
  Serial.println(myData.buttonIndex);
  Serial.print("Board: ");
  Serial.print(myData.board);
  Serial.print(" Button: ");
  Serial.println(myData.button);
  delay(waittime);  // Adjust delay as needed
  turnOffAllLEDs(5);
}

void readCurrent(){
for (int i = 0; i < s_or_d; i++){
powerread[i] = analogRead(powerPins[i]);
voltages[i] = vcc/ADCmax*powerread[i]*vratio;
if(currentreadout == 1){
Serial.print("V");
Serial.print(i+1);
Serial.print(" ");
Serial.print(voltages[i]);
Serial.print(" | ");}
}
current = (vss-voltages[0])/Rsense;
if(currentreadout == 1){
Serial.print(" Current ");
Serial.println(current);}

if(current > currentthresh){
  Serial.println("Current Trigger");
  
}
}
