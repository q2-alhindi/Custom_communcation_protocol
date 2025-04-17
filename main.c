#include "protocol.h"
#include <stdio.h>

int main() {
    protocol_init();
    int conn = protocol_connect(NULL, 0);
    char buffer[256];

    printf("Pico is running in echo mode. Send messages to echo back.\n");

    while (1) {
        int bytes_received = protocol_receive(conn, buffer, sizeof(buffer));
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Received: %s\n", buffer);
            protocol_send(conn, buffer, bytes_received);
        }
    }

    protocol_disconnect(conn);
    protocol_cleanup();
    return 0;
}