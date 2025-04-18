cmake_minimum_required(VERSION 3.10)

# Import the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set the project name
project(SensorDataProtocol C CXX ASM)

# Initialize the Pico SDK
pico_sdk_init()

# Add your executable target
add_executable(transmitter
    main.c
    protocol.c
)

# Enable USB output, UART output, etc.
pico_enable_stdio_usb(transmitter 1)
pico_enable_stdio_uart(transmitter 1)

# Link Pico standard libraries
target_link_libraries(transmitter
    pico_stdlib
    hardware_uart
)

# Create .uf2, .bin, etc.
pico_add_extra_outputs(transmitter)
