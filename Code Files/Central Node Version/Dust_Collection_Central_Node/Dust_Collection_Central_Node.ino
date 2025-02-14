#include <esp_now.h>
#include <WiFi.h>
#define number_control_boards 3
const int NUM_PEERS = number_control_boards+1;
#define analogpulse 0
const uint8_t broadcastAddresses[NUM_PEERS][6] = {
  {0xF0, 0xF5, 0xBD, 0x2C, 0xFF, 0x80}, // #3  Tablesaw Board
  {0xF0, 0xF5, 0xBD, 0x2D, 0x03, 0x58}, //#2   Chopsaw Board
  {0xF0, 0xF5, 0xBD, 0x2D, 0x14, 0x4C}, // #1  Bandsaw board
  {0xF0, 0xF5, 0xBD, 0x2C, 0xF9, 0xE0},  //DC V2*/
};

typedef struct struct_message {
  bool CollectorState;
  bool ExtraState;
  int button;
  int board;
  bool reboot;
} struct_message;

struct_message myData;
int value = 0;
int last_active = 10;
const int DC_led = 6;
const int LED_binds[3][4] = {
    
      {9,5,45,45},
        {1,2,3,4},
            {7,8,45,45},
};

int t1 = 0;
int t2 = 0;
bool new_states = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(DC_led,OUTPUT);
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 4; j++){
      pinMode(LED_binds[i][j],OUTPUT);
    }
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

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


  Serial.println("Setup Complete");
}



void loop() {
blink_active();
static int light = 0;
static int interval = 5;
  // put your main code here, to run repeatedly:
if(myData.CollectorState == 0){
  analogWrite(DC_led,0);
}
else{
  if(millis() > t1 + 50){
    t1 = millis();
    light += interval;
    if(light >= 255){
      light = 255;
      interval = interval * -1;
    }
    if(light <= 0){
      light = 0;
      interval = interval * -1;
    }
    analogWrite(DC_led,light);
    if(analogpulse == 1){
      analogWrite(value,light);}
    else{
      digitalWrite(value,1);
    }
  }
}
if(new_states == true && millis() > t2 + 2000){
  new_states = false;
  push_new_states();
}}

void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if(myData.reboot == true){
    Serial.println("Received reboot command");
    esp_restart();
  }
  t2 = millis();
  new_states = true;
  last_active = 10;
  Serial.print("DC State ");
  Serial.println(myData.CollectorState);
  Serial.print("Board: ");
  Serial.print(myData.board);
  Serial.print(" Button: ");
  Serial.println(myData.button);
  status_update();
  if(myData.board != 0){}

  else{
    Serial.println("Dust Remote Triggered");
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void status_update(){
wipe_status();

value = LED_binds[myData.board-1][myData.button-1];
Serial.print("Pin activated: ");
Serial.println(value);

}

void wipe_status(){
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 4; j++){
      if(analogpulse == 1){
      analogWrite(LED_binds[i][j],0);
    }
    else{
      digitalWrite(LED_binds[i][j],0);
    }}
  }
}

void push_new_states(){
  Serial.println("Sending New States");
  for (int j = 0; j < NUM_PEERS; j++) {
    if(j != myData.board-1){
    //Serial.print("Sending To: ");
    //Serial.println(j+1);
    esp_now_send(broadcastAddresses[j], (uint8_t *) &myData, sizeof(myData));
    delay(50);}}
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