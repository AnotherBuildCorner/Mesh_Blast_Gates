#define NUM_BUTTONS 2
#define NUM_SERVOS 1
#define adcPin 2
#define maxang 270
#define minang 0
#define VDC 3.3
#define adc0 2280
#define adcmax 3690
#define vratio 2
#include <ESP32Servo.h>
const int buttonPins[NUM_BUTTONS] = {0, 1};  // Change as per your setup
const int servoPins[NUM_SERVOS] = {21};        // Change as per your setup
Servo servos[NUM_SERVOS]; 


void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);

  }
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(adcPin,INPUT);
}

void loop() {
static int angle = 0;
bool x = digitalRead(buttonPins[0]);
bool y = digitalRead(buttonPins[1]);
int z = analogRead(adcPin);
float current = ((adc0-z)*1000*vratio)/adcmax;
if(x == HIGH){
  angle += 1;
//  Serial.print("Position: ");
//  Serial.println(angle);
  if(angle >= maxang){
    angle = maxang;
  }
}
if(y == HIGH){
  angle -= 1;

  if(angle <= minang){
    angle = minang;
  }
}
if(current > 0){
Serial.print("Position: ");
Serial.print(angle);
Serial.print(" Amps: ");
Serial.println(current);}
servos[0].write(angle);
delay(20);
}
