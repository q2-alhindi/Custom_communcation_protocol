# Worksheet 2 - Custom Communication Protocol

## Overview
This project implements a custom communication protocol between a Raspberry Pi Pico (developed in C) and a host PC application (developed in Python) utilising UART for data transmission. The system is designed to support robust and structured data transmission over USB Serial, enabling reliable communication for use cases like sensor data exchange, echo testing, and protocol validation.

## Getting Started
To get a copy of this project up and running on your local machine, follow these instructions.

## Essentials
* Raspberry Pi Pico SDK installed and configured
* CMake (>3.10)
* Make 
* USB connection to the Pico (For uploading .uf2 file)
* Pyserial for serial communication
* Virtual COM port tool to simulate serial bridges

## Initialising Project Environment
### Cloning the pico SDK
~~~bash
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
~~~
Then set the environment variable so CMake can find it:
~~~bash
export PICO_SDK_PATH=/path/to/pico-sdk
~~~
Replace /path/to/pico-sdk with the actual path where you cloned it.

### Cloning and building the project
Clone the repository:
~~~bash
    git clone https://gitlab.uwe.ac.uk/q2-alhendi/worksheett2.git
~~~

To build and compile the C files follow these steps:
~~~bash
    mkdir build  # Create a build directory     
    cd build # navigate into build directory
    cmake .. # Generate build files 
    make # Compile project
~~~

### Running the Project
To run the project, follow these steps in order:
1. Hold the BOOTSEL button on the Raspberry Pi Pico and connect it to your computer via USB.
2. Open the build folder where the compiled .uf2 file is located.
3. Drag and drop the .uf2 file into the Pico's folder (which appears automatically).
4. Start the serial bridge (to route communication between the Pico and virtual COM ports)
~~~bash
python connector.py
~~~
5. Run
~~~bash
python client.py
~~~
6. Start sending messages in the terminal, you will see responses echoed back from the Pico.

You can also open Tera Term (or any serial terminal) on the second virtual COM port to observe communication live from a different tool.

### File Structure
**Hierarchical representation**
```bash
    Pico Side
    ├── protocol.c
    ├── protocol.h       
    ├── main.c   
    ├── CMakeLists.txt 
    ├── pico_sdk_import.cmake                  
```
```bash
    Python Side
    ├── client.py
    ├── connector.py       
```
## Implementation Details
### Pico Side
**protocol.h**: This is the header file for **protocol.c**.

**Snippet 1**:
```c
#define MAX_PACKET_SIZE 255
#define HEADER_BYTE 0xAA     
```
`MAX_PACKET_SIZE` defines the maximum number of bytes that can be included in the data payload of a packet. It is set to 255.

`HEADER_BYTE` is a special byte (0xAA) placed at the beginning of every packet to signal the start of a new message. It helps the receiver correctly identify and synchronize with incoming data.

**Snippet 2**:
~~~c
void protocol_init(void);
int protocol_connect(const char* address, int port);
int protocol_send(int connectionHandle, const void* data, int dataSize);
int protocol_receive(int connectionHandle, void* buffer, int bufferSize);
void protocol_disconnect(int connectionHandle);
void protocol_cleanup(void);
~~~
This snippet shows the declaration of all the functions.

**protocol.c**:
This file includes the core logic of the custom communication protocol on the Raspberry Pi Pico. It handles serial initialization, packet construction (including header, length, payload, and checksum), as well as receiving and validating incoming packets.

**Snippet 1**:
~~~c
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
~~~
The `protocol_init` function initializes the USB serial connection on the Raspberry Pi Pico. It ensures the communication interface is only set up once by checking a flag `is_initialized`. It waits until the USB is fully connected before proceeding, with a short delay to allow the host system to detect the device.

**Snippet 2**:
~~~c
int protocol_connect(const char* address, int port) {
    if (!is_initialized) {
        protocol_init();
    }
    return 1;
}
~~~
`protocol_connect` simply ensures that the protocol is initialized by calling `protocol_init` if connection is not initialised and then returns a dummy connection handle to simulate a successful connection.

**Snippet 3**:
~~~c
static uint8_t calculate_checksum(Packet* packet) {
    uint8_t checksum = 0;
    checksum ^= packet->header;
    checksum ^= packet->length;
    for (int i = 0; i < packet->length; i++) {
        checksum ^= packet->data[i];
    }
    return checksum;
}
~~~










