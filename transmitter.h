#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MAX_PACKET_SIZE 255   // Maximum payload size
#define HEADER_BYTE 0xAA      // Packet header byte

typedef struct {
    uint8_t header;                   // HEADER_BYTE
    uint8_t length;                   // Length of the data payload
    uint8_t data[MAX_PACKET_SIZE];   // Data payload
    uint8_t checksum;                // XOR checksum
} Packet;

// Initializes the communication system (e.g., USB Serial)
void protocol_init(void);

// Establishes a connection (dummy for USB Serial, always returns 1)
int protocol_connect(const char* address, int port);

// Sends data using the protocol packet structure
int protocol_send(int connectionHandle, const void* data, int dataSize);

// Receives data and validates packet integrity
int protocol_receive(int connectionHandle, void* buffer, int bufferSize);

// Disconnects the connection (no action for USB Serial)
void protocol_disconnect(int connectionHandle);

// Cleans up the protocol module (resets internal state)
void protocol_cleanup(void);

#endif