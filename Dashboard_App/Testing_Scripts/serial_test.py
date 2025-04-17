import serial
import time

def arduinoTest(port = '/dev/serial0', baud = 115200, message = 'Hello'):
    ser = serial.Serial(port, baudrate = baud, timeout = 1)
    print(f"Opened {ser.name}")

    time.sleep(0.1)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    count = 0
    while True:
        # Send "Hello {n}\n"
        outgoing = f"{message} {count}\n".encode('utf-8')
        print(f"TX: {outgoing!r}")
        ser.write(outgoing)

        response = ser.readline()
        print(f"RX: {response!r}")

        count = (count + 1) % 100

        time.sleep(0.1)

if __name__ == '__main__':
    arduinoTest()
