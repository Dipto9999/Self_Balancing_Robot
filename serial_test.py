import serial
import time

# Configure the serial port
ser = serial.Serial(
    port='/dev/ttyS0',  # UART port on Raspberry Pi
    baudrate=115200,
    timeout=1
)

try:
    while True:
        # Send data to Arduino
        message = "Hello from Raspberry Pi\n"
        ser.write(b'A')

        # Read response
        if ser.in_waiting > 0:
            response = ser.readline().decode('utf-8').rstrip()
            print(f"Arduino says: {response}")

        time.sleep(5)
except KeyboardInterrupt:
    print("Communication stopped")
    ser.close()