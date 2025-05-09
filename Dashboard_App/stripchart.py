import os
import serial

import pandas as pd
import numpy as np
import math

import tkinter as tk

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.animation as animation

import datetime as dt
import pytz

from IPython.display import display
class ArduinoSerial(serial.Serial) :
    PORT = '/dev/serial0'
    BAUDRATE = 115200
    def __init__(self, port = PORT, baudrate = 115200):
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

        while not self.isOpen() :
            try:
                self.open()
            except:
                pass
        print('Serial Port is Open')

class StripChart:
    REQUEST_BYTE = b'A'
    def __init__(self, master, conn = None, data_size = 25, sample_rate = 0.1, ylim = 30):
        self.master = master
        self.conn = conn
        self.fig, self.ax = plt.subplots(figsize = (900 / 100, 755 / 100))

        self.data_size = data_size # Number of Samples to Display
        self.sample_rate = sample_rate
        self.ylim = ylim # Y-Axis Limits (Deg)

        self.refresh() # Refresh Data

        self.figures_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Figures")
        self.logbook_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Logbook")

        os.makedirs(self.figures_dir, exist_ok = True)
        os.makedirs(self.logbook_dir, exist_ok = True)

    def start(self, conn):
        """Start StripChart."""
        self.conn = conn
        self.animation = animation.FuncAnimation(
            fig = self.fig, # Figure
            func = self.update, # Update Function
            frames = self.data_size, # Number of Frames
            blit = False, # Prevent Re-rendering Entire Plot
            interval = self.sample_rate * 1000, # Delay in ms
        )
        self.canvas = FigureCanvasTkAgg(self.fig, master = self.master)
        self.canvas_widget = self.canvas.get_tk_widget()

    def update(self, frame):
        """Update StripChart with New Data."""
        self.rx_angle()

        # Update Plot Data
        self.accelerometer_line.set_data(self.sample_data, self.accelerometer_data)
        self.gyroscope_line.set_data(self.sample_data, self.gyroscope_data)
        self.complementary_line.set_data(self.sample_data, self.complementary_data)

        # Adjust x Limits to Scroll Forward
        if (len(self.sample_data) > 0) and (self.sample_data[-1] > self.data_size + 1):
            self.ax.set_xlim(
                self.sample_data[0], # Start at First Data Point
                self.sample_data[0] + 1.25 * self.data_size # Display 25% More Data
            )
        else:
            self.ax.set_xlim(1, 1.25 * self.data_size)

    def refresh(self):
        """Refresh Data from StripChart."""
        self.sample_data = []
        self.accelerometer_data = []
        self.gyroscope_data = []
        self.complementary_data = []

        self.data_df = pd.DataFrame(
            columns = [
                'Time (PST)',
                'Accelerometer Angle (Deg)',
                'Gyroscope Angle (Deg)',
                'Complementary Angle (Deg)'
            ]
        )

        self.ax.cla()

        self.ax.set_title('Real-Time Angle Strip-Chart')
        self.ax.grid(True)

        self.ax.set_xlim(1, 1.25 * self.data_size)
        self.ax.xaxis.set_major_locator(plt.MultipleLocator(5))

        self.ax.set_ylim(-self.ylim, self.ylim) # Angle Expected to be Between -180 Deg and 180 Deg
        self.ax.set_yticks(np.arange(-self.ylim, self.ylim + 1, 5))

        self.ax.tick_params(axis = 'both', labelsize = 8)

        self.ax.set_xlabel('Sample (#)')
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

    def stop(self):
        """Set Connection to None and Stop Updating."""
        if self.conn is not None:
            self.conn.close()
            self.conn = None

        if hasattr(self, 'animation') and self.animation:
            self.animation.event_source.stop()
            self.animation = None

    def save(self):
        fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
        self.save_logs(fig_name)
        self.save_fig(fig_name)

    def rx_angle(self):
        """Read Angle Data from Serial Connection."""
        def decode(incoming) :
            """Read Data from Serial Connection."""
            decoded = None
            if (self.conn is not None) and (incoming is not None):
                try:
                    decoded = str(incoming.decode('ascii')).strip('\r').strip('\n')
                except UnicodeDecodeError:
                    decoded = None
            return decoded

        if self.conn is None:
            return

        # Request Data from Arduino
        try:
            # print("TX Byte:", StripChart.REQUEST_BYTE)
            self.conn.write(StripChart.REQUEST_BYTE)
        except serial.SerialException:
            self.conn.reconnect()

        try:
            incoming = self.conn.readline()
            # print(f"Incoming Data: {incoming}")

            decoded = decode(incoming)
            # print(f"Decoded Data: {decoded}")

            if decoded is None:
                raise ValueError(f"Received Invalid: {incoming}")

            arduinoStream: list = str(decode(incoming)).split(' ')
            if len(arduinoStream) != 3:
                 raise ValueError

            accelerometer_angle = float(arduinoStream[0])
            gyroscope_angle = float(arduinoStream[1])
            complementary_angle = float(arduinoStream[2])

            print(f"Accelerometer Angle: {accelerometer_angle} (Deg)")
            print(f"Gyroscope Angle: {gyroscope_angle} (Deg)")
            print(f"Complementary Angle: {complementary_angle} (Deg)")

            # Append Data
            new_sample = (self.sample_data[-1] + 1) if self.sample_data else 1

            self.sample_data.append(new_sample)
            self.accelerometer_data.append(accelerometer_angle)
            self.gyroscope_data.append(gyroscope_angle)
            self.complementary_data.append(complementary_angle)

            if self.data_df.empty:
                self.data_df = pd.DataFrame({
                    'Time (PST)': dt.datetime.now(pytz.timezone('US/Pacific')),
                    'Accelerometer Angle (Deg)': accelerometer_angle,
                    'Gyroscope Angle (Deg)': gyroscope_angle,
                    'Complementary Angle (Deg)': complementary_angle
                }, index = [0])
            else:
                self.data_df = pd.concat([
                    self.data_df,
                    pd.DataFrame([{
                        'Time (PST)': dt.datetime.now(pytz.timezone('US/Pacific')),
                        'Accelerometer Angle (Deg)': accelerometer_angle,
                        'Gyroscope Angle (Deg)': gyroscope_angle,
                        'Complementary Angle (Deg)': complementary_angle
                    }])], ignore_index = True
                )
        except serial.SerialException:
            self.conn.reconnect()
        except ValueError as value_error: # Parse Error
            display("Parse Error:", str(value_error))

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
        file_path = os.path.join(self.logbook_dir, f"{file_name}.csv")

        csv_df = self.data_df.copy()
        csv_df['Time (PST)'] = self.data_df['Time (PST)'].dt.strftime('%H:%M:%S.%f').str[:-3]
        csv_df.to_csv(file_path, index = False)

        print(f"Data Exported to {file_path}\n\nData Preview:\n")
        display(self.data_df.head(2))
        display("--------------------")
        display(self.data_df.tail(2))

class StripchartApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Stripchart App")
        # self.geometry("1000x850")
        # self.resizable(False, False)

        self.dashboard_frame = tk.Frame(self, bg = '#000000')
        self.serial_frame = tk.Frame(self.dashboard_frame)
        self.stripchart_frame = tk.Frame(self.dashboard_frame)

        ############################
        ### Serial Frame Widgets ###
        ############################

        self.port_label = tk.Label(self.serial_frame, text = "Serial Line : ", bg = '#787882')
        self.port_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')
        self.port_entry.insert(0, ArduinoSerial.PORT)

        self.baudrate_label = tk.Label(self.serial_frame, text = "Speed : ", bg = '#787882')
        self.baudrate_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')
        self.baudrate_entry.insert(0, str(ArduinoSerial.BAUDRATE))

        self.port_entry.bind("<Return>", lambda event: self.open_serial())
        self.baudrate_entry.bind("<Return>", lambda event: self.open_serial())
        self.open_button = tk.Button(
            self.serial_frame, text = "Open", command = self.open_serial, bg = '#6e9eeb',
        )

        self.port_label.grid(
            row = 0, column = 0,
            columnspan = 2,
            padx = 10, pady = 10,
            sticky = tk.EW
        )
        self.port_entry.grid(
            row = 0, column = 2,
            columnspan = 1,
            padx = 10, pady = 10,
            sticky = tk.EW
        )

        self.baudrate_label.grid(
            row = 0, column = 3,
            columnspan = 2,
            padx = 10, pady = 10,
            sticky = tk.EW
        )
        self.baudrate_entry.grid(
            row = 0, column = 5,
            columnspan = 1,
            padx = 10, pady = 10,
            sticky = tk.EW
        )

        self.open_button.grid(
            row = 0, column = 6,
            columnspan = 2,
            padx = 10, pady = 10,
            sticky = tk.EW
        )

        self.serial_frame.grid_rowconfigure(0, weight = 1)

        self.serial_frame.grid_columnconfigure(0, weight = 1)
        self.serial_frame.grid_columnconfigure(1, weight = 1)
        self.serial_frame.grid_columnconfigure(2, weight = 0)

        self.serial_frame.grid_columnconfigure(3, weight = 1)
        self.serial_frame.grid_columnconfigure(4, weight = 1)
        self.serial_frame.grid_columnconfigure(5, weight = 0)

        self.serial_frame.grid_columnconfigure(6, weight = 2)

        ################################
        ### StripChart Frame Widgets ###
        ################################

        self.stripchart = StripChart(self.stripchart_frame)
        self.save_button = tk.Button(
            self.stripchart_frame, text = "Save", command = self.save_data, bg = '#6e9eeb',
        )

        self.stripchart.canvas_widget.grid(row = 0, column = 0)
        self.save_button.grid(
            row = 1, column = 0,
            padx = 10, pady = 10,
            sticky = tk.EW
        )

        # Position Widgets

        self.serial_frame.grid(row = 0, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.stripchart_frame.grid(row = 1, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)

        self.dashboard_frame.grid_rowconfigure(0, weight = 1)
        self.dashboard_frame.pack(fill = tk.BOTH, expand = True)

        self.protocol("WM_DELETE_WINDOW", self.on_close) # Cleanup

    def open_serial(self):
        port = self.port_entry.get()
        baudrate = self.baudrate_entry.get()

        try :
            self.conn = ArduinoSerial(
                port = port, baudrate = int(baudrate)
            )

            self.stripchart.start(self.conn) # Start StripChart
            self.stripchart.canvas_widget.grid(row = 0, column = 0)
            self.open_button.config(text = "Close", command = self.close_serial, bg = '#f06e47')
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None
            self.open_button.config(text = "Open", command = self.open_serial, bg = '#6e9eeb')

    def close_serial(self):
        if self.stripchart.conn is not None:
            self.stripchart.stop()
            self.stripchart.refresh() # Refresh Data
            self.open_button.config(text = "Open", command = self.open_serial, bg = '#6e9eeb')

    def save_data(self):
        if not self.stripchart.data_df.empty:
            self.stripchart.save()
        else:
            print("No Data Available")

    def on_close(self):
        """Close Serial Connection on Application Exit."""
        self.close_serial()
        self.save_data()

        # Exit Application
        self.destroy()

if __name__ == "__main__":
    StripchartApp().mainloop()