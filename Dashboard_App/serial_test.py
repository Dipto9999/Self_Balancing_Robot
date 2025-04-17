import serial
import time

def loopback(port='/dev/serial0', baud = 115200, message = 'Hello'):
    ser = serial.Serial(port, baudrate = baud, timeout = 1)
    print(f"Opened {ser.name}")

    time.sleep(0.1)

    # Flush Existing Data
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    for i in range(100):
        outgoing = bytes(f"{message} {i}\n", 'utf-8')
        # Send the test message
        print(f"Sending: {outgoing!r}")
        ser.write(outgoing)

        # Read Upto 100 Bytes or Until Timeout
        response = ser.read(100)
        print(f"Received: {response!r}")

    ser.close()
    print("Port Closed.")

if __name__ == '__main__':
    loopback()
