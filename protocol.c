#include "pico/stdlib.h"
#include "protocol.h"
#include <string.h>
#include <stdio.h>

// Global flag to ensure the protocol is only initialized once
static int is_initialized = 0;

// Initializes the USB Serial communication system
// Waits for a connection from the host PC before proceeding
void protocol_init(void) {
    if (!is_initialized) {
        stdio_init_all(); // Initialize USB and UART stdio
        while (!stdio_usb_connected()) {
            sleep_ms(100); // Wait for the host to connect to the Pico
        }
        sleep_ms(1000); // Give some extra time for the connection to stabilize
        is_initialized = 1; // Mark the protocol as initialized
    }
}

// Establishes a mock connection; real networking isn't needed with USB serial
// Ensures initialization is complete before returning a dummy connection handle
int protocol_connect(const char* address, int port) {
    if (!is_initialized) {
        protocol_init(); // Make sure USB is ready
    }
    return 1; // Return a fixed handle since USB doesn't use address/port
}

// Calculates a simple XOR checksum from the packet's header, length, and data
// This helps ensure data integrity during transmission
static uint8_t calculate_checksum(Packet* packet) {
    uint8_t checksum = 0;
    checksum ^= packet->header;
    checksum ^= packet->length;
    for (int i = 0; i < packet->length; i++) {
        checksum ^= packet->data[i];
    }
    return checksum;
}

// Constructs and sends a data packet over USB Serial
int protocol_send(int connectionHandle, const void* data, int dataSize) {
    // Check if data size is within valid bounds
    if (dataSize > MAX_PACKET_SIZE || dataSize < 1) {
        printf("Send failed: Invalid dataSize %d\n", dataSize);
        return -1;
    }

    // Build the packet
    Packet packet;
    packet.header = HEADER_BYTE;                // Add start-of-packet marker
    packet.length = (uint8_t)dataSize;          // Store payload length
    memcpy(packet.data, data, dataSize);        // Copy the data into the packet
    packet.checksum = calculate_checksum(&packet); // Calculate checksum

    // Create raw byte stream to send
    uint8_t buffer[MAX_PACKET_SIZE + 3];        // 1 byte header, 1 length, N data, 1 checksum
    buffer[0] = packet.header;
    buffer[1] = packet.length;
    memcpy(buffer + 2, packet.data, packet.length);
    buffer[2 + packet.length] = packet.checksum;

    // Send each byte individually using putchar_raw
    for (int i = 0; i < packet.length + 3; i++) {
        int result = putchar_raw(buffer[i]);
        if (result == PICO_ERROR_TIMEOUT || result == PICO_ERROR_GENERIC) {
            printf("Transmission failed at byte %d\n", i);
            return -1;
        }
    }

    // Debug output
    printf("Sent packet: header=0x%02X, length=%d, checksum=0x%02X\n", packet.header, packet.length, packet.checksum);
    return dataSize; // Return number of bytes sent (payload only)
}

// Receives a packet, validates it, and copies the payload into a buffer
int protocol_receive(int connectionHandle, void* buffer, int bufferSize) {
    if (bufferSize <= 0) {
        printf("Receive failed: Invalid bufferSize %d\n", bufferSize);
        return -1;
    }

    Packet packet;
    uint32_t start_time = time_us_32(); // Start timer for timeout tracking

    // Step 1: Wait for the header byte
    packet.header = getchar_timeout_us(5000000);
    if (packet.header != HEADER_BYTE) {
        return -1; // Not a valid start byte
    }

    // Step 2: Read the length byte
    packet.length = getchar_timeout_us(5000000);
    if (packet.length > MAX_PACKET_SIZE || packet.length > bufferSize) {
        printf("Invalid length: %d\n", packet.length);
        return -1;
    }

    // Step 3: Read the payload bytes
    for (int i = 0; i < packet.length; i++) {
        // Timeout after 10 seconds max
        if ((time_us_32() - start_time) / 1000000 > 10) {
            printf("Timeout receiving packet data\n");
            return -1;
        }
        packet.data[i] = getchar_timeout_us(5000000);
    }

    // Step 4: Read the checksum byte
    if ((time_us_32() - start_time) / 1000000 > 10) {
        printf("Timeout receiving checksum\n");
        return -1;
    }
    packet.checksum = getchar_timeout_us(5000000);

    // Step 5: Verify checksum
    if (calculate_checksum(&packet) != packet.checksum) {
        printf("Checksum mismatch\n");
        return -1;
    }

    // Step 6: Copy the payload into the provided buffer
    memcpy(buffer, packet.data, packet.length);
    return packet.length; // Return number of bytes received
}

// Disconnect function – not needed for USB, so left empty
void protocol_disconnect(int connectionHandle) {
    // No operation required for USB Serial
}

// Resets the initialization flag (optional cleanup)
void protocol_cleanup(void) {
    is_initialized = 0;
}
