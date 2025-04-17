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
python receiver.py
~~~
6. Start sending messages in the terminal, you will see responses echoed back from the Pico.

You can also open Tera Term (or any serial terminal) on the second virtual COM port to observe communication live from a different tool.



