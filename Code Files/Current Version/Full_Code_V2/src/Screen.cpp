#include "Screen.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SSD1306_I2C_ADDRESS 0x3C // Define the I2C address for the SSD1306


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initializeScreen() {
    // Initialize the screen
    Serial.begin(115200);
    delay(1000);
    Serial.println("Screen initialized");

  //  Wire.begin(22, 23); // Initialize I2C with pins 22 (SDA) and 23 (SCL)

    if(!display.begin(SSD1306_SWITCHCAPVCC,SSD1306_I2C_ADDRESS, OLED_RESET)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.display();
    delay(2000);
    display.clearDisplay();
}

void updateScreen(const char* message) {
    // Update the screen with the provided message
    Serial.println(message);
}

bool parseBoardData(const Board_Data& data, int& shortIndex, int& shortValue, int& longIndex, int& longValue) {
    bool foundShort = false;
    bool foundLong = false;

    for (int i = 0; i < NUM_PEERS; ++i) {
        if (data.ShortPress[i] != 0 && !foundShort) {
            shortIndex = i;
            shortValue = data.ShortPress[i];
            foundShort = true;
        }
        if (data.LongPress[i] != 0 && !foundLong) {
            longIndex = i;
            longValue = data.LongPress[i];
            foundLong = true;
        }
        if (foundShort && foundLong) {
            return true;
        }
    }
    return foundShort || foundLong;
}

void convertAndSelectGateName(const Board_Data& data, char* shortGateName, char* longGateName) {
    int shortIndex, shortValue, longIndex, longValue;
    if (parseBoardData(data, shortIndex, shortValue, longIndex, longValue)) {
        bool shortPressArray[NUM_BUTTONS] = {0};
        bool longPressArray[NUM_BUTTONS] = {0};

        mapPressToBinary(shortValue, shortPressArray);
        mapPressToBinary(longValue, longPressArray);

        if (shortIndex < NUM_GATE_BOARDS && shortValue != 0) {
            for (int i = 0; i < NUM_BUTTONS; ++i) {
                if (shortPressArray[i]) {
                    strcpy(shortGateName, GateNames[shortIndex][i]);
                    break;
                }
            }
        }

        if (longIndex < NUM_GATE_BOARDS && longValue != 0) {
            for (int i = 0; i < NUM_BUTTONS; ++i) {
                if (longPressArray[i]) {
                    strcpy(longGateName, GateNames[longIndex][i]);
                    break;
                }
            }
        }
    }
}

void startCountdownTimer(const Board_Data& data) {
    int shortIndex, shortValue, longIndex, longValue;
    char shortGateName[20] = "";
    char longGateName[20] = "";
    if (parseBoardData(data, shortIndex, shortValue, longIndex, longValue)) {
        unsigned long countdownTime = 0;

        if (longValue != 0) {
            countdownTime = data.LongTimer * 60 * 1000; // Convert to milliseconds
        } else if (shortValue != 0) {
            countdownTime = data.ShortTimer * 60 * 1000; // Convert to milliseconds
        }

        if (countdownTime > 0) {
            unsigned long startTime = millis();
            while (millis() - startTime < countdownTime) {
                // Perform countdown actions here
                // For example, update a display or perform other tasks
                unsigned long timeLeft = countdownTime - (millis() - startTime);
                displayOLED(shortGateName, longGateName, timeLeft);
            }
            // Countdown complete, perform any necessary actions
        }
    }
}

void displayOLED(const char* shortGateName, const char* longGateName, int countdownTime) {
    static unsigned long shutdownscreen = 0;
    static bool shutdown = false;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Long Press Gate: ");
    display.println(longGateName);

    display.setCursor(0, 20);
    display.println("Short Press Gate: ");
    display.println(shortGateName);

    
    int minutes = countdownTime / 60;
    int seconds = (countdownTime % 60);

    if(countdownTime == 0){
        display.setCursor(40, 40);
        display.setTextSize(2);
        display.println("OFF");
        if(shutdown == false){
            shutdownscreen = millis();
            shutdown = true;
        }
        if(millis() > shutdownscreen + 10000 && shutdown == true){
            display.clearDisplay();
        }
    }
    else{
    display.setCursor(40, 40);
    display.setTextSize(2);
    display.print(minutes);
    display.print(":");
    display.println(seconds);
    shutdown = false;
    }

    display.display();
}

bool screenhandler(bool reset_countdown){
    static unsigned long screentimer = 0;
    static int countdownTime = 0;
    if (millis() > screentimer + 1000) {
        char shortGateName[20] = "";
        char longGateName[20] = "";
        convertAndSelectGateName(BoardData, shortGateName, longGateName); 
        if(reset_countdown == true){
            
            if (longGateName[0] != '\0') {
                countdownTime = BoardData.LongTimer * 60; // Convert to seconds
            } else if (shortGateName[0] != '\0') {
                countdownTime = BoardData.ShortTimer * 60; // Convert to seconds
            } else {
                countdownTime = 0;
            }
            reset_countdown = false;}
        screentimer = millis();
        countdownTime = countdownTime - 1;

        if(countdownTime < 0 ){
            countdownTime = 0;
        }    
    
    displayOLED(shortGateName, longGateName, countdownTime);
}
return reset_countdown;
}
