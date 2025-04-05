import time
import sys
import traceback
import multiprocessing as mp
import serial

def serial_run(data_queue, stop_event, port='/dev/ttyS0', baudrate=115200):
    try:
        conn = serial.Serial(port=port, baudrate=baudrate)
        conn.reset_input_buffer()
    except Exception as e:
        print("[StripChartProcess] Failed to open serial.")
        traceback.print_exc()
        return

    try:
        while not stop_event.is_set():
            try:
                conn.write(b'A')
            except serial.SerialException:
                break

            line = conn.readline().decode('ascii', errors='ignore').strip()
            if line:
                data_queue.put(line)

            time.sleep(0.1)

    except Exception as e:
        traceback.print_exc()
    finally:
        conn.close()
        print("[StripChartProcess] Exiting stripchart process.")
