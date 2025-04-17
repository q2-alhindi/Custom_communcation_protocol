import serial  # For serial communication
import time    # For delays and timing

# Define the actual serial ports for your setup
pico_port = 'COM5'        # Pico USB serial port
virtual_port1 = 'COM8'    # Virtual COM port for Python script to listen to
virtual_port2 = 'COM10'   # Virtual COM port for Tera Term or another tool

# Function to connect to a serial port with retries
def connect_port(port, baudrate):
    max_attempts = 3  # Retry up to 3 times
    for attempt in range(max_attempts):
        try:
            # Attempt to open the serial port
            ser = serial.Serial(port, baudrate, timeout=1)
            ser.flushInput()   # Clear input buffer
            ser.flushOutput()  # Clear output buffer
            return ser
        except serial.SerialException as e:
            print(f"Failed to connect to {port} (attempt {attempt + 1}/{max_attempts}): {e}")
            time.sleep(1)  # Wait a second before retrying
    # If it fails after max_attempts, raise an error
    raise serial.SerialException(f"Could not connect to {port} after {max_attempts} attempts")

# Try to open all required ports
try:
    pico = connect_port(pico_port, 9600)
    virtual1 = connect_port(virtual_port1, 9600)
    virtual2 = connect_port(virtual_port2, 9600)
    print(f"Bridge started: {pico_port} <-> {virtual_port1} and {virtual_port2}")
except serial.SerialException as e:
    print(f"Error opening ports: {e}")
    exit(1)

# Main bridging loop
try:
    while True:
        try:
            # If Pico sent any data, forward it to both virtual ports
            if pico.in_waiting:
                data = pico.read(min(pico.in_waiting, 1024))  # Read available data, max 1024 bytes
                virtual1.write(data)
                virtual2.write(data)
                virtual1.flush()
                virtual2.flush()
                print(f"Pico -> Virtual: {data}")

            # If virtual1 (used by Python app) sends data, forward it to the Pico
            if virtual1.in_waiting:
                data = virtual1.read(min(virtual1.in_waiting, 1024))
                pico.write(data)
                pico.flush()
                print(f"Virtual1 -> Pico (Python): {data}")

            # If virtual2 (used by Tera Term) sends data, forward it to the Pico
            if virtual2.in_waiting:
                data = virtual2.read(min(virtual2.in_waiting, 1024))
                pico.write(data)
                pico.flush()
                print(f"Virtual2 -> Pico (Tera Term): {data}")

            time.sleep(0.01)  # Short delay to prevent CPU overuse
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            time.sleep(1)  # Wait a bit before retrying in case of error

# Graceful shutdown on CTRL+C
except KeyboardInterrupt:
    print("\nStopping bridge...")

# Always close the ports at the end
finally:
    pico.close()
    virtual1.close()
    virtual2.close()
    print("Ports closed.")
