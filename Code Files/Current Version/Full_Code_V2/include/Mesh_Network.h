#ifndef MESH_NETWORK_H
#define MESH_NETWORK_H

#include "Primary.h"
#include <esp_now.h>
#include <WiFi.h>

// Remove the redefinition of NUM_PEERS
// #define NUM_PEERS 10 // Total Number of Endpoints, and collector boards

// Update struct_message definition to Board_Data
typedef struct Board_Data {
    bool CollectorState;
    bool ExtraState;
    int button;
    int board;
    bool reboot;
    int LongPress[NUM_PEERS];
    int ShortPress[NUM_PEERS];
} Board_Data;

extern Board_Data BoardData;
extern bool new_data_recv; // Declare the new boolean flag

void EndpointDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void EndpointDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len);
void CentralNodeDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void CentralNodeDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len);
void LaunchEndpoints();
void LaunchCentralNode();
void startup_push();
void mapPressToBinary(int value, bool GateStatusArray[4]);
int mapBinaryToPress(const bool GateStatusArray[4]);
void sendGateStatusToCentralNode(bool GateStatusLongPress[4], bool GateStatusShortPress[4]);
void resetGateStatusArrays();
void push_data(); // Declare the new function
void checkPressAndSetCollectorState(); // Declare the new function
void PrintGateArrays();
void PrintLocalGateArrays();
#endif // MESH_NETWORK_H
