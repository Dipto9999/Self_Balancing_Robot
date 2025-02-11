import threading as td
import queue

import time
from arduinoSerial import *

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.clock import Clock

from kivy_garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg
import matplotlib.pyplot as plt

import pandas as pd
import numpy as np
import datetime as dt

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

        self.ax.set_ylim(0, self.ylim) # Angle Expected to be Between 0° and 360°
        self.ax.set_yticks(range(0, self.ylim + 1, 15)) # Set Y-Ticks to 15° Intervals

        self.ax.set_xlabel('Time (s)')
        self.ax.set_ylabel('Angle (°)')

        self.accelerometer_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Accelerometer Angle (°)', color = 'lightblue')
        self.gyroscope_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Gyroscope Angle (°)', color = 'tomato')
        self.complementary_line, = self.ax.plot([], [], lw = 1.5, label = 'Complementary Angle (°)', color = 'purple')

        self.fig.subplots_adjust(bottom = 0.2)
        self.ax.legend(
            loc = 'upper center',
            bbox_to_anchor = (0.5, -0.2), # Position Above Plot with Padding
            ncol = 3, framealpha = 0.9,  # 3 Columns, Transparent Background
            fontsize = 10,
        )

        # Create a Thread-Safe Queue for Serial Data
        self.data_queue = queue.Queue()
        self.stop_event = td.Event()

        self.thread = td.Thread(target = self.rx_angle, daemon = True)
        self.thread.start()

        self.missed = 0 # Track Missed Data

    def update(self):
        """Update Strip Chart with New Data."""
        if not self.data_queue.empty(): # Process All Data in Queue
            try:
                arduinoStream = self.data_queue.get().split(' ')

                # print(f"Incoming Value: {arduinoStream}")

                if len(arduinoStream) != 3:
                    raise ValueError

                accelerometer_angle: float = float(arduinoStream[0])
                gyroscope_angle: float = float(arduinoStream[1])
                complementary_angle = float(arduinoStream[2])

                # print(f"Accelerometer Angle: {accelerometer_angle}°")
                # print(f"Gyroscope Angle: {gyroscope_angle}°")
                # print(f"Complementary Angle: {complementary_angle}°")

                # Append Data
                if len(self.sample_data) > 0:
                    self.sample_data.append(self.sample_data[-1]+1)
                else:
                    self.sample_data.append(1)
                self.accelerometer_data.append(accelerometer_angle)
                self.gyroscope_data.append(gyroscope_angle)
                self.complementary_data.append(complementary_angle)
            except ValueError:
                pass
        else:
            self.missed += 1

        if self.missed > 100: # Stop Updating After 10s of Missed Data
            self.conn.reconnect()
            self.stop()
            self.stop_event.clear()

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
                print(f"Incoming Value: {arduinoStream}")

                self.data_queue.put(arduinoStream)
                print("Data Queue Size:", self.data_queue.qsize())
            except serial.SerialException:
                self.conn.reconnect()
            except ValueError:
                pass

            time.sleep(StripChart.SAMPLE_RATE) # Read Data Every 100ms

    def save_fig(self, fig_name) :
        """Save Figure to File."""
        file_path = f"Angles/Figures/{fig_name}.jpg"
        self.fig.savefig(file_path, format = 'jpg', dpi = 800)
        print(f"Figure Saved to {file_path}")

    def save_logs(self, fig_name):
        """Save Data to CSV File."""
        data_df = pd.DataFrame({
            'Sample' : self.sample_data,
            'Accelerometer Angle (°)' : self.accelerometer_data,
            'Gyroscope Angle (°)' : self.gyroscope_data,
            'Complementary Angle (°)' : self.complementary_data
        })

        file_path = f"Angles/LogBook/{fig_name}.csv"
        data_df.to_csv(file_path, index = False)

        print(f"Data Exported to {file_path}\n\nData Preview:\n")
        display(data_df.head(2))

class DashboardPageLayout(GridLayout):
    def __init__(self, app, **kwargs):
        super().__init__(**kwargs)
        self.app = app

        self.cols = 1
        self.spacing = 10
        self.padding = 10

        self.strip_chart = StripChart(conn = self.app.conn)
        self.canvas_widget = FigureCanvasKivyAgg(self.strip_chart.fig)
        self.add_widget(self.canvas_widget)

        Clock.schedule_interval(self.update_chart, StripChart.SAMPLE_RATE) # Update Plot Every 100ms

    def update_chart(self, dt):
        """Update Strip Chart."""
        self.strip_chart.update() # Update Strip Chart
        self.canvas_widget.draw_idle() # Update Canvas

class TestApp(App):
    def build(self):
        # self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        # td.Thread(target = self._start_async_loop, daemon = True).start() # Start Event Loop in Separate Thread

        # Serial Connection
        try:
            self.conn = ArduinoSerial(port = 'COM3', baudrate = 9600)
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None
        # Configure Window
        Window.size = (1000, 800)
        Window.resizable = False

        self.title = "Arduino Bluetooth Driver"
        self.dashboard_page = DashboardPageLayout(app = self)
        return self.dashboard_page

    def on_stop(self):
        """Close Serial Connection on Application Exit."""
        if self.conn is not None and self.conn.isOpen():
            self.conn.close()

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.dashboard_page.strip_chart.save_logs(fig_name)
            self.dashboard_page.strip_chart.save_fig(fig_name)

if __name__ == "__main__":
    TestApp().run()