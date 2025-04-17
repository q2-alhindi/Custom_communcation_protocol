import serial  # Module to handle serial communication
import time    # Module for delays and timing

# This class defines our custom serial communication protocol
class CustomProtocol:
    HEADER_BYTE = 0xAA  # Start-of-packet marker

    # Constructor to set up the serial port and communication settings
    def __init__(self, port='COM9', baudrate=9600):
        self.port = port                  # COM port to connect to
        self.baudrate = baudrate          # Baud rate for communication
        self.serial = None                # Will hold the serial connection
        self.running = False              # Tracks if connection is active
        self.last_received = None         # Stores last received data (not used here)
        print(f"Initializing with port {self.port} and baudrate {self.baudrate}")

    # Establish the serial connection
    def connect(self):
        print("Attempting to connect...")
        try:
            # Open serial port
            self.serial = serial.Serial(self.port, self.baudrate, timeout=1)
            self.running = True
            print("Connected successfully!")
            self.serial.flushInput()     # Clear input buffer
            self.serial.flushOutput()    # Clear output buffer
            return True
        except serial.SerialException as e:
            print(f"Failed to connect: {e}")
            return False

    # Generate a checksum using XOR logic (based on header and data)
    def _calculate_checksum(self, data, length):
        checksum = self.HEADER_BYTE ^ length
        for byte in data[:length]:
            checksum ^= byte
        return checksum

    # Send a message to the Pico device
    def send(self, data):
        if not self.serial or not self.serial.is_open:
            print("Serial port not open!")
            return False
        # Build packet: header + length + data + checksum
        packet = bytearray([self.HEADER_BYTE, len(data)]) + data
        checksum = self._calculate_checksum(data, len(data))
        packet.append(checksum)
        self.serial.write(packet)
        self.serial.flush()
        return True

    # Receive a message from the Pico device
    def receive(self, buffer_size):
        if not self.serial or not self.serial.is_open:
            return None

        while True:
            header = self.serial.read(1)  # Read first byte
            if len(header) == 0:
                return None
            if header[0] != self.HEADER_BYTE:
                continue  # Wait until we find the correct start byte

            length_bytes = self.serial.read(1)
            if len(length_bytes) == 0:
                return None
            length = length_bytes[0]

            # Ignore packets that are too large
            if length > buffer_size:
                continue

            data = self.serial.read(length)      # Read the data bytes
            checksum = self.serial.read(1)       # Read the checksum byte
            if len(data) != length or len(checksum) != 1:
                continue  # Retry if packet is incomplete

            # Validate checksum
            if self._calculate_checksum(data, length) != checksum[0]:
                continue  # Retry if checksum is invalid

            return data  # If everything is valid, return the received data

    # Close the serial connection
    def cleanup(self):
        if self.serial and self.serial.is_open:
            self.serial.close()
            print("Serial connection closed.")

# This section runs when the file is executed directly
if __name__ == "__main__":
    # Create an instance of the protocol using COM9 (adjust this to your setup)
    protocol = CustomProtocol('COM9', 9600)

    # Try to connect to the Pico
    if protocol.connect():
        try:
            time.sleep(2)  # Give the Pico time to get ready
            print("\nEcho mode activated. Pico will echo back your messages.")
            # Main loop to send and receive messages
            while True:
                user_message = input("Enter message to send (or 'exit' to quit): ").strip()
                if user_message.lower() == 'exit':
                    break  # Exit the loop if user types "exit"
                if not user_message:
                    continue  # Ignore empty messages

                # Send the user's message
                protocol.send(user_message.encode())

                # Wait for and display the Pico's response
                response = protocol.receive(255)
                if response:
                    print("Pico echoed:", response.decode(errors='ignore'))
                else:
                    print("No valid response received.")
        except KeyboardInterrupt:
            print("\nExiting on user interrupt...")
        finally:
            # Clean up and close the connection when done
            protocol.cleanup()
    else:
        print("Failed to initialize protocol.")
