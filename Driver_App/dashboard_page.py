import asyncio
import threading as td
import serial

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.clock import Clock

from kivy_garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg
import matplotlib.pyplot as plt

import pandas as pd
import datetime as dt
from IPython.display import display

class Serial(serial.Serial) :
    def __init__(self, port = 'COM3', baudrate = 9600):
        super().__init__(port = port, baudrate = baudrate)

        if self.isOpen() :
            print('Serial Port is Open')
        else:
            print('Serial Port is Closed')

    def close(self) :
        super().close()
        print('Serial Port is Closed')

    def reconnect(self) :
        self.close()

        while not self.isOpen() :
            try:
                self.open()
            except:
                pass
        print('Serial Port is Open')

class StripChart:
    def __init__(self, conn, data_size = 50, ylim = 360):
        self.conn = conn
        self.fig, self.ax = plt.subplots(figsize = (900 / 100, 755 / 100))

        self.data_size, self.ylim = data_size, ylim

        self.t_data = []
        self.accelerometer_data = []
        self.gyroscope_data = []
        self.complementary_data = []

        self.ax.set_title('Real-Time Angle Strip-Chart')
        self.ax.grid(True)

        self.ax.set_ylim(0, self.ylim) # Angle Expected to be Between 0° and 360°
        self.ax.set_yticks(range(0, self.ylim + 1, 15)) # Set Y-Ticks to 15° Intervals

        self.ax.set_xlabel('Samples')
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

    def update(self):
        """Update Strip Chart with New Data."""
        self.rx_angle() # Read Angle Data from Serial

        self.accelerometer_line.set_data(self.t_data, self.accelerometer_data)
        self.gyroscope_line.set_data(self.t_data, self.gyroscope_data)
        self.complementary_line.set_data(self.t_data, self.complementary_data)

        if (len(self.t_data) > 0) and (self.t_data[-1] > self.data_size):
            self.ax.set_xlim(self.t_data[0], self.t_data[0] + round(1.25 * self.data_size))
        else:
            self.ax.set_xlim(0, round(1.25 * self.data_size))

    def rx_angle(self):
        """Read Angle Data from Serial Connection."""
        def read_serial() :
            """Read Data from Serial Connection."""
            if (self.conn is not None) :
                return self.conn.readline().decode('ascii').strip('\r').strip('\n')
            else :
                return None
        try:
            self.conn.write(b'A') # Request Serial Transmission of Angle
        except serial.SerialException:
            self.conn.reconnect()

        try:
            arduinoStream: list = str(read_serial()).rstrip('\r').split(' ')

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
            if len(self.t_data) > 0:
                self.t_data.append(self.t_data[-1]+1)
            else:
                self.t_data.append(1)
            self.accelerometer_data.append(accelerometer_angle)
            self.gyroscope_data.append(gyroscope_angle)
            self.complementary_data.append(complementary_angle)

            # Limit Size to Trailing Data
            self.t_data = self.t_data[-self.data_size:]
            self.accelerometer_data = self.accelerometer_data[-self.data_size:]
            self.gyroscope_data = self.gyroscope_data[-self.data_size:]
            self.complementary_data = self.complementary_data[-self.data_size:]
        except serial.SerialException:
            self.conn.reconnect()
        except ValueError:
            pass

    def save_fig(self, fig_name) :
        """Save Figure to File."""
        file_path = f"Angles/Figures/{fig_name}.jpg"
        self.fig.savefig(file_path, format = 'jpg', dpi = 800)
        print(f"Figure Saved to {file_path}")

    def save_logs(self, fig_name):
        """Save Data to CSV File."""
        data_df = pd.DataFrame({
            'Sample' : self.t_data,
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

        try:
            self.conn = Serial(port = 'COM3', baudrate = 9600)
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None

        self.strip_chart = StripChart(conn = self.conn)
        self.canvas_widget = FigureCanvasKivyAgg(self.strip_chart.fig)
        self.add_widget(self.canvas_widget)

        Clock.schedule_interval(self.update_chart, 0.05) # Update Plot Every 50ms

    def update_chart(self, dt):
        """Update Strip Chart."""
        self.strip_chart.update() # Update Strip Chart
        self.canvas_widget.draw_idle() # Update Canvas

class TestApp(App):
    def build(self):
        # self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        # td.Thread(target = self._start_async_loop, daemon = True).start() # Start Event Loop in Separate Thread

        # Configure Window
        Window.size = (1000, 800)
        Window.resizable = False
        self.title = "Arduino Bluetooth Driver"
        self.dashboard_page = DashboardPageLayout(app = self)
        return self.dashboard_page

    def on_stop(self):
        """Close Serial Connection on Application Exit."""
        if self.dashboard_page.conn is not None and self.dashboard_page.conn.isOpen():
            self.dashboard_page.conn.close()

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.dashboard_page.strip_chart.save_logs(fig_name)
            self.dashboard_page.strip_chart.save_fig(fig_name)

    # def _start_async_loop(self):
    #     """Start Asyncio Event Loop."""
    #     asyncio.set_event_loop(self.async_loop)
    #     self.async_loop.run_forever()

if __name__ == "__main__":
    TestApp().run()