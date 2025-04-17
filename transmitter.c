#include "pico/stdlib.h"
#include "transmitter.h"
#include <string.h>
#include <stdio.h>

// Global flag to track if the protocol has been initialized
static int is_initialized = 0;

// Initializes the USB Serial communication system
void protocol_init(void) {
    if (!is_initialized) {
        stdio_init_all();
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }
        sleep_ms(1000);
        is_initialized = 1;
    }
}

// Establishes a connection
int protocol_connect(const char* address, int port) {
    if (!is_initialized) {
        protocol_init();
    }
    return 1;
}

// Calculates the XOR checksum for a packet
static uint8_t calculate_checksum(Packet* packet) {
    uint8_t checksum = 0;
    checksum ^= packet->header;
    checksum ^= packet->length;
    for (int i = 0; i < packet->length; i++) {
        checksum ^= packet->data[i];
    }
    return checksum;
}

// Sends a packet over USB Serial
int protocol_send(int connectionHandle, const void* data, int dataSize) {
    if (dataSize > MAX_PACKET_SIZE || dataSize < 1) {
        printf("Send failed: Invalid dataSize %d\n", dataSize);
        return -1;
    }

    Packet packet;
    packet.header = HEADER_BYTE;
    packet.length = (uint8_t)dataSize;
    memcpy(packet.data, data, dataSize);
    packet.checksum = calculate_checksum(&packet);

    uint8_t buffer[MAX_PACKET_SIZE + 3];
    buffer[0] = packet.header;
    buffer[1] = packet.length;
    memcpy(buffer + 2, packet.data, packet.length);
    buffer[2 + packet.length] = packet.checksum;

    for (int i = 0; i < packet.length + 3; i++) {
        int result = putchar_raw(buffer[i]);
        if (result == PICO_ERROR_TIMEOUT || result == PICO_ERROR_GENERIC) {
            printf("Transmission failed at byte %d\n", i);
            return -1;
        }
    }

    printf("Sent packet: header=0x%02X, length=%d, checksum=0x%02X\n", packet.header, packet.length, packet.checksum);
    return dataSize;
}

// Receives a packet over USB Serial and validates its integrity
int protocol_receive(int connectionHandle, void* buffer, int bufferSize) {
    if (bufferSize <= 0) {
        printf("Receive failed: Invalid bufferSize %d\n", bufferSize);
        return -1;
    }

    Packet packet;
    uint32_t start_time = time_us_32();
    packet.header = getchar_timeout_us(5000000);
    if (packet.header != HEADER_BYTE) {
        return -1;
    }

    packet.length = getchar_timeout_us(5000000);
    if (packet.length > MAX_PACKET_SIZE || packet.length > bufferSize) {
        printf("Invalid length: %d\n", packet.length);
        return -1;
    }

    for (int i = 0; i < packet.length; i++) {
        if ((time_us_32() - start_time) / 1000000 > 10) {
            printf("Timeout receiving packet data\n");
            return -1;
        }
        packet.data[i] = getchar_timeout_us(5000000);
    }

    if ((time_us_32() - start_time) / 1000000 > 10) {
        printf("Timeout receiving checksum\n");
        return -1;
    }
    packet.checksum = getchar_timeout_us(5000000);

    if (calculate_checksum(&packet) != packet.checksum) {
        printf("Checksum mismatch\n");
        return -1;
    }

    memcpy(buffer, packet.data, packet.length);
    return packet.length;
}

// Placeholder for disconnecting the connection
void protocol_disconnect(int connectionHandle) {
    // No operation required
}

// Cleans up the protocol module
void protocol_cleanup(void) {
    is_initialized = 0;
}