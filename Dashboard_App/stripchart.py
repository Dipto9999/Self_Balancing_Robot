import os
import serial

import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.animation as animation

from IPython.display import display
class ArduinoSerial(serial.Serial) :
    def __init__(self, port = '/dev/ttyS0', baudrate = 115200):
        super().__init__(port = port, baudrate = baudrate)

    def open(self) :
        super().open()
        if self.isOpen() :
            print('Serial Port is Open')
        else:
            print('Serial Port is Closed')

    def close(self) :
        super().close()
        print('Serial Port is Closed')

    def reconnect(self) :
        self.close()
        self.open()
        return self.isOpen()

class StripChart:
    SAMPLE_RATE = 0.1 # 100ms
    def __init__(self, master, conn = None, data_size = 50, ylim = 180):
        self.master = master
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

        self.ax.set_ylim(-self.ylim, self.ylim) # Angle Expected to be Between -180 Deg and 180 Deg
        self.ax.set_yticks(np.arange(-self.ylim, self.ylim + 1, 15))

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

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.master)
        self.canvas_widget = self.canvas.get_tk_widget()

        self.figures_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Figures")
        self.logbook_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Logbook")

        os.makedirs(self.figures_dir, exist_ok = True)
        os.makedirs(self.logbook_dir, exist_ok = True)

        self.missed = 0 # Track Missed Data

    def start(self, conn):
        """Start StripChart."""
        self.conn = conn
        self.animation = animation.FuncAnimation(
            fig = self.fig, # Figure
            func = self.update, # Update Function
            frames = self.data_size, # Number of Frames
            blit = False, # Prevent Re-rendering Entire Plot
            interval = StripChart.SAMPLE_RATE * 1000, # Delay in ms
        )
        self.canvas = FigureCanvasTkAgg(self.fig, master = self.master)
        self.canvas_widget = self.canvas.get_tk_widget()

    def update(self):
        """Update StripChart with New Data."""
        self.rx_angle()

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
        """Set Connection to None and Stop Updating."""
        self.missed, self.conn = 0, None

    def rx_angle(self):
        """Read Angle Data from Serial Connection."""
        def read_serial() :
            """Read Data from Serial Connection."""
            if (self.conn is not None) :
                return str(self.conn.readline().decode('ascii').strip('\r').strip('\n'))
            else :
                return None

        # Loop Until Connection is Closed or Stop Event is Set
        while (self.conn is not None):
            # Request Serial Transmission of Angle
            try:
                self.conn.write(b'A')
            except serial.SerialException:
                if not self.conn.reconnect():
                    self.stop()

            # Read Angle Data
            try:
                arduinoStream: list = str(read_serial()).rstrip('\r').split(' ')
                # print(f"RX Value: {arduinoStream}")

                if len(arduinoStream) != 3:
                    raise ValueError

                # print(f"Incoming Value: {arduinoStream}")

                accelerometer_angle: float = float(arduinoStream[0])
                gyroscope_angle: float = float(arduinoStream[1])
                complementary_angle = float(arduinoStream[2])

                print(f"Accelerometer Angle: {accelerometer_angle}°")
                print(f"Gyroscope Angle: {gyroscope_angle}°")
                print(f"Complementary Angle: {complementary_angle}°")

                # Append Data
                if len(self.sample_data) > 0:
                    self.sample_data.append(self.sample_data[-1]+1)
                else:
                    self.sample_data.append(0)
                self.accelerometer_data.append(accelerometer_angle)
                self.gyroscope_data.append(gyroscope_angle)
                self.complementary_data.append(complementary_angle)
            except ValueError:
                self.missed += 1

        if self.conn and self.missed > 100: # Stop Updating After 10s of Missed Data
            if not self.conn.reconnect(): # Reconnect Serial Connection
                self.stop() # Stop Updating

        # Limit Size to Trailing Data
        self.sample_data = self.sample_data[-self.data_size:]
        self.accelerometer_data = self.accelerometer_data[-self.data_size:]
        self.gyroscope_data = self.gyroscope_data[-self.data_size:]
        self.complementary_data = self.complementary_data[-self.data_size:]

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