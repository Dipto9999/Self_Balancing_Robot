import threading as td
import queue

import os
import serial

import time
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

from IPython.display import display

class StripChart:
    SAMPLE_RATE = 0.1 # 100ms
    def __init__(self, conn, data_size = 50, ylim = 360):
        self.conn = conn
        self.fig, self.ax = plt.subplots(figsize = (900 / 100, 755 / 100))

        self.data_size, self.ylim = data_size, ylim

        # Angle Data
        self.sample_data = []
        self.accelerometer_data = []
        self.gyroscope_data = []
        self.complementary_data = []

        self.ax.set_title('Real-Time Angle Strip-Chart')
        self.ax.grid(True)

        self.ax.set_xlim(
            0, # Start at 0s
            1.25 * self.data_size * StripChart.SAMPLE_RATE # End at 25% More Data
        )
        self.ax.set_xticks(
            np.arange(
                0,  # Start at 0s
                1.25 * self.data_size * StripChart.SAMPLE_RATE, # End at 25% More Data
                0.5  # Set Ticks to 0.5s Intervals
            )
        )

        self.ax.set_ylim(0, self.ylim) # Angle Expected to be Between 0 Deg and 360 Deg
        self.ax.set_yticks(range(0, self.ylim + 1, 15)) # Set Y-Ticks to 15 Deg Intervals

        self.ax.tick_params(axis = 'both', labelsize = 8)

        self.ax.set_xlabel('Time (s)')
        self.ax.set_ylabel('Angle (Deg)')

        self.accelerometer_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Accelerometer Angle (Deg)', color = 'lightblue')
        self.gyroscope_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Gyroscope Angle (Deg)', color = 'tomato')
        self.complementary_line, = self.ax.plot([], [], lw = 1.5, label = 'Complementary Angle (Deg)', color = 'purple')

        self.fig.subplots_adjust(bottom = 0.2)
        self.ax.legend(
            loc = 'upper center',
            bbox_to_anchor = (0.5, -0.15), # Position Above Plot with Padding
            ncol = 3, framealpha = 0.9,  # 3 Columns, Transparent Background
            fontsize = 10,
        )

        # Create a Thread-Safe Queue for Serial Data
        self.data_queue = queue.Queue()
        self.stop_event = td.Event()

        self.figures_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Figures")
        self.logbook_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Logbook")

        os.makedirs(self.figures_dir, exist_ok = True)
        os.makedirs(self.logbook_dir, exist_ok = True)

        self.thread = td.Thread(target = self.rx_angle, daemon = True)
        self.thread.start()

        self.missed = 0 # Track Missed Data

    def update(self):
        """Update Strip Chart with New Data."""
        if not self.data_queue.empty(): # Process All Data in Queue
            try:
                arduinoStream = self.data_queue.get().split(' ')

                # print(f"Queue Value: {arduinoStream}")

                if len(arduinoStream) != 3:
                    raise ValueError

                accelerometer_angle: float = float(arduinoStream[0])
                gyroscope_angle: float = float(arduinoStream[1])
                complementary_angle = float(arduinoStream[2])

                # print(f"Accelerometer Angle: {accelerometer_angle} Deg")
                # print(f"Gyroscope Angle: {gyroscope_angle} Deg")
                # print(f"Complementary Angle: {complementary_angle} Deg")

                # Append Data
                if len(self.sample_data) > 0:
                    self.sample_data.append(self.sample_data[-1]+1)
                else:
                    self.sample_data.append(0)
                self.accelerometer_data.append(accelerometer_angle)
                self.gyroscope_data.append(gyroscope_angle)
                self.complementary_data.append(complementary_angle)
            except ValueError:
                pass
        else:
            self.missed += 1

        if self.conn and self.missed > 100: # Stop Updating After 10s of Missed Data
                self.conn.reconnect() # Reconnect Serial Connection
                self.stop() # Stop Serial Thread
                self.stop_event.clear() # Clear Stop Event

                # Restart Serial Thread
                self.thread = td.Thread(target = self.rx_angle, daemon = True)
                self.thread.start()

        # Limit Size to Trailing Data
        self.sample_data = self.sample_data[-self.data_size:]
        self.accelerometer_data = self.accelerometer_data[-self.data_size:]
        self.gyroscope_data = self.gyroscope_data[-self.data_size:]
        self.complementary_data = self.complementary_data[-self.data_size:]

        # Update Plot Data
        t_data = [t * 0.1 for t in self.sample_data]
        self.accelerometer_line.set_data(t_data, self.accelerometer_data)
        self.gyroscope_line.set_data(t_data, self.gyroscope_data)
        self.complementary_line.set_data(t_data, self.complementary_data)

        # Adjust x Limits to Scroll Forward
        if (len(self.sample_data) > 0) and (self.sample_data[-1] > self.data_size):
            self.ax.set_xlim(
                t_data[0], # Start at First Data Point
                t_data[0] + 1.25 * self.data_size * StripChart.SAMPLE_RATE # End at 25% More Data
            ) # Display 25% More Data
            self.ax.set_xticks(
                np.arange(
                    t_data[0], # Start at First Data Point
                    t_data[0] + 1.25 * self.data_size * StripChart.SAMPLE_RATE, # End at 25% More Data
                    0.5  # Set Ticks to 0.5s Intervals
                )
            )
        else:
            self.ax.set_xlim(
                0, # Start at 0s
                1.25 * self.data_size * StripChart.SAMPLE_RATE # End at 25% More Data
            )
            self.ax.set_xticks(
                np.arange(
                    0,  # Start at 0s
                    1.25 * self.data_size * StripChart.SAMPLE_RATE, # End at 25% More Data
                    0.5  # Set Ticks to 0.5s Intervals
                )
            )

    def stop(self):
        """Signal Serial Thread to Stop and Wait for Thread to Finish."""
        self.stop_event.set()
        self.thread.join()

        self.missed = 0

    def rx_angle(self):
        """Read Angle Data from Serial Connection."""
        def read_serial() :
            """Read Data from Serial Connection."""
            if (self.conn is not None) :
                return self.conn.readline().decode('ascii').strip('\r').strip('\n')
            else :
                return None

        while not self.stop_event.is_set(): # Loop Until Stop Event is Set
            # Request Serial Transmission of Angle
            try:
                self.conn.write(b'A')
            except serial.SerialException:
                self.conn.reconnect()

            # Read Angle Data
            try:
                arduinoStream = str(read_serial()).rstrip('\r')
                # print(f"RX Value: {arduinoStream}")

                # print("Data Queue Size:", self.data_queue.qsize())
                self.data_queue.put(arduinoStream) # Add New Data
            except serial.SerialException:
                self.conn.reconnect()
            except ValueError:
                pass

            time.sleep(StripChart.SAMPLE_RATE) # Read Data Every 100ms

    def save_fig(self, fig_name) :
        """Save Figure to File."""
        file_path = os.path.join(self.figures_dir, f"{fig_name}.jpg")

        self.fig.savefig(file_path, format = 'jpg', dpi = 800)
        print(f"Figure Saved to {file_path}")

    def save_logs(self, file_name):
        """Save Data to CSV File."""
        data_df = pd.DataFrame({
            'Sample' : self.sample_data,
            'Accelerometer Angle (Deg)' : self.accelerometer_data,
            'Gyroscope Angle (Deg)' : self.gyroscope_data,
            'Complementary Angle (Deg)' : self.complementary_data
        })

        file_path = os.path.join(self.logbook_dir, f"{file_name}.csv")
        data_df.to_csv(file_path, index = False)

        print(f"Data Exported to {file_path}\n\nData Preview:\n")
        display(data_df.head(2))