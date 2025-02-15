#include "Mesh_Network.h"
#include "Primary.h" // Include Primary.h to resolve undefined reference
#include "Settings.h" // Include Settings.h to access MAC addresses

// Ensure extern variables are declared
extern int last_active;

Board_Data BoardData; // Define BoardData here
bool new_data_recv = false; // Initialize the new boolean flag

void EndpointDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void EndpointDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
    memcpy(&BoardData, incomingData, sizeof(BoardData));
    if (BoardData.reboot == true) {
        Serial.println("Received reboot command");
        esp_restart();
    }

    last_active = 10; // Highlight the usage of last_active
    if(BoardData.board <= NUM_GATE_BOARDS){
        Serial.print("DC State");
        Serial.println(BoardData.CollectorState);
        Serial.print(GateNames[BoardData.board][BoardData.button]);
        Serial.println(" Actuated");}

    // Convert integer values to binary representation and write to GateStatus arrays
    for (int i = 0; i < NUM_GATE_BOARDS; i++) {
        mapPressToBinary(BoardData.ShortPress[i], GateStatusShortPress);
        mapPressToBinary(BoardData.LongPress[i], GateStatusLongPress);
    }

    new_data_recv = true; // Set the new_data_recv flag to true
}

void CentralNodeDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void CentralNodeDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
    memcpy(&BoardData, incomingData, sizeof(BoardData));
    if (BoardData.reboot == true) {
        Serial.println("Received reboot command");
        esp_restart();
    }

    last_active = 10; // Highlight the usage of last_active
    if(BoardData.board <= NUM_GATE_BOARDS){
    Serial.print("DC State");
    Serial.println(BoardData.CollectorState);
    Serial.print(GateNames[BoardData.board][BoardData.button]);
    Serial.println(" Actuated");}
    PrintGateArrays();

    new_data_recv = true; // Set the new_data_recv flag to true
}

void push_data() {
    if (new_data_recv) {
        checkPressAndSetCollectorState();
        for (int i = 0; i < NUM_PEERS; i++) {
            esp_now_send(EndpointAddresses[i], (uint8_t *) &BoardData, sizeof(BoardData));
        }
        new_data_recv = false; // Reset the flag after sending data
    }
}

void checkPressAndSetCollectorState() {
    for (int i = 0; i < NUM_GATE_BOARDS; i++) {
        if (BoardData.ShortPress[i] != 0 || BoardData.LongPress[i] != 0) {
            BoardData.CollectorState = true;
            return;
        }
    }
    BoardData.CollectorState = false;
}

void initializeMeshNetworkData(Board_Data &data) {
    // Initialize the new arrays
    for (int i = 0; i < NUM_PEERS; ++i) {
        data.LongPress[i] = 0;
        data.ShortPress[i] = 0;
    }
    // ...existing code...
}

void LaunchEndpoints() {
    Serial.println(BoardLabelArray[BoardSel]);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(EndpointDataSent);
    esp_now_register_recv_cb(EndpointDataRecv);

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, CentralNodeAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void LaunchCentralNode() {
    Serial.println(BoardLabelArray[BoardSel]);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(CentralNodeDataSent);
    esp_now_register_recv_cb(CentralNodeDataRecv);

    for (int i = 0; i < NUM_PEERS; i++) {
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, EndpointAddresses[i], 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer");
            return;
        }
    }
}

void startup_push() {
    if (BoardTypeArray[BoardSel] == center) { // Central Node
        Serial.println("sending from Central Node");
        for (int i = 0; i < NUM_PEERS; i++) {
            esp_now_send(EndpointAddresses[i], (uint8_t *) &BoardData, sizeof(BoardData));
        }
    } else { // Endpoint
        Serial.println("sending from Endpoint");
        esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
    }
}

void mapPressToBinary(int value, bool GateStatusArray[4]) {
    for (int i = 0; i < 4; ++i) {
        GateStatusArray[i] = (value >> i) & 1;
    }
}

int mapBinaryToPress(const bool GateStatusArray[4]) {
    int value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= (GateStatusArray[i] << i);
    }
    return value;
}

void sendGateStatusToCentralNode(bool GateStatusLongPress[4], bool GateStatusShortPress[4]) {
    BoardData.board = BoardSel; // Set BoardData.board to BoardSel
    BoardData.LongPress[BoardSel] = mapBinaryToPress(GateStatusLongPress);
    BoardData.ShortPress[BoardSel] = mapBinaryToPress(GateStatusShortPress);
    esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
}

void resetGateStatusArrays() {
    for (int i = 0; i < NUM_SERVOS; ++i) {
        BoardData.LongPress[i] = 0;
        BoardData.ShortPress[i] = 0;
    }
    for (int i = NUM_SERVOS; i < NUM_PEERS; ++i) {
        BoardData.LongPress[i] = 0;
        BoardData.ShortPress[i] = 0;
    }
}

void PrintGateArrays(){
    Serial.print("Global Long Press States | (");
    for(int i = 0; i < NUM_PEERS; i++){
        Serial.print(BoardData.LongPress[i]);
        Serial.print(", ");
    }
    Serial.println(")");
    Serial.print("Global Short Press States | (");
    for(int i = 0; i < NUM_PEERS; i++){
        Serial.print(BoardData.ShortPress[i]);
        Serial.print(", ");
    }
    Serial.println(")");
}

void PrintLocalGateArrays(){
    Serial.print("Local  Long Press States | (");
    for(int i = 0; i < NUM_SERVOS; i++){
        Serial.print(GateStatusLongPress[i]);
        Serial.print(", ");
    }
    Serial.println(")");
    Serial.print("Local  Short Press States | (");
    for(int i = 0; i < NUM_SERVOS; i++){
        Serial.print(GateStatusShortPress[i]);
        Serial.print(", ");
    }
    Serial.println(")");
}
