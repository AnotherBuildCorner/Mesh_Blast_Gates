#include "Mesh_Network.h"
#include "Primary.h" // Include Primary.h to resolve undefined reference

// Ensure extern variables are declared
extern int last_active;

const uint8_t CentralNodeAddress[6] = {0x8C, 0xBF, 0xEA, 0xCF, 0x7F, 0x00};//{0xC8, 0x2E, 0x18, 0xF1, 0x2B, 0xA4};
const uint8_t EndpointAddresses[NUM_PEERS][6] = {
    {0x8C, 0xBF, 0xEA, 0xCF, 0x82, 0x3C},
    {0x8C, 0xBF, 0xEA, 0xCF, 0x75, 0x44},
    // Add other endpoint addresses here
};

Board_Data BoardData; // Define BoardData here

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
    Serial.print("DC State");
    Serial.println(BoardData.CollectorState);
    Serial.print("Board: ");
    Serial.print(BoardData.board);
    Serial.print(" Button: ");
    Serial.println(BoardData.button);

    if (BoardData.board != 0) {
        delay(waittime);
    } else {
        Serial.println("Dust Remote Triggered");
    }
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
    Serial.print("DC State");
    Serial.println(BoardData.CollectorState);
    Serial.print("Board: ");
    Serial.print(BoardData.board);
    Serial.print(" Button: ");
    Serial.println(BoardData.button);

    if (BoardData.board != 0) {
        delay(waittime);
    } else {
        Serial.println("Dust Remote Triggered");
    }
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
    if (BoardType == 3) { // Central Node
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

        BoardData.LongPress[BoardSel] = mapBinaryToPress(GateStatusLongPress);
        BoardData.ShortPress[BoardSel] = mapBinaryToPress(GateStatusShortPress);
    esp_now_send(CentralNodeAddress, (uint8_t *) &BoardData, sizeof(BoardData));
}

void resetGateStatusArrays() {
    for (int i = 0; i < NUM_PEERS; ++i) {
        BoardData.LongPress[i] = 0;
        BoardData.ShortPress[i] = 0;
    }
}