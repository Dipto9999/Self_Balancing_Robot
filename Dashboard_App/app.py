import sys
import threading as td

import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

import datetime as dt
import serial

from camera import CameraDisplay
from stripchart import ArduinoSerial, StripChart

class Dashboard:
    def __init__(self, master):
        self.master = master

        self.dashboard_frame = tk.Frame(self.master, bg = '#000000')

        self.camera_frame = tk.Frame(self.dashboard_frame, bg = '#141654')
        self.diagnostic_frame = tk.Frame(self.dashboard_frame, bg = '#48484d')

        self.serial_frame = tk.Frame(self.diagnostic_frame, bg = '#787882')
        self.stripchart_frame = tk.Frame(self.diagnostic_frame, bg = '#48484d')

        ############################
        ### Camera Frame Widgets ###
        ############################

        self.button_frame = tk.Frame(self.camera_frame, bg = '#787882')

        self.record_button = tk.Button(
            self.button_frame,
            text = "Stop", bg = "red", width = 15,
            command = self.toggle_record
        )
        self.snapshot_button = tk.Button(
            self.button_frame,
            text = "Snapshot", width = 15,
            command = lambda: self.take_snapshot()
        )

        self.cam_feed = CameraDisplay(self.camera_frame) # Camera Feed
        # self.cam_feed = tk.Label(self.camera_frame, bg = '#141654')

        self.record_button.pack(side = tk.LEFT, padx = 5, pady = 5)
        self.snapshot_button.pack(side = tk.RIGHT, padx = 5, pady = 5)

        self.button_frame.grid(row = 0, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.cam_feed.grid(row = 1, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)

        self.camera_frame.grid_rowconfigure(0, weight = 0)
        self.camera_frame.grid_rowconfigure(1, weight = 0)

        self.button_frame.grid_columnconfigure(0, weight = 0)
        self.button_frame.grid_columnconfigure(1, weight = 0)

        ############################
        ### Serial Frame Widgets ###
        ############################

        self.port_label = tk.Label(self.serial_frame, text = "Serial Line : ", bg = '#787882')
        self.port_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')
        self.port_entry.insert(0, ArduinoSerial.PORT)

        self.baudrate_label = tk.Label(self.serial_frame, text = "Speed : ", bg = '#787882')
        self.baudrate_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')
        self.baudrate_entry.insert(0, str(ArduinoSerial.BAUDRATE))

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

        self.serial_frame.grid(row = 0, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.stripchart_frame.grid(row = 1, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)

        # Position Widgets

        self.camera_frame.grid(row = 0, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.diagnostic_frame.grid(row = 0, column = 1, padx = 10, pady = 10, sticky = tk.NSEW)

        self.dashboard_frame.pack(fill = tk.BOTH, expand = True)

        self.cam_feed.start_recording()
        self.master.after(500, self.update_feed)

    def toggle_record(self):
        # pass
        if self.cam_feed.filename != "": # Recording
            self.cam_feed.stop_recording()
            self.record_button.config(text = "Start", bg = "green")
        else: # Not Recording
            self.cam_feed.start_recording()
            self.record_button.config(text = "Stop", bg = "red")

    def update_feed(self):
        self.cam_feed.update()
        self.master.after(int(CameraDisplay.SAMPLE_RATE * 10E3), self.update_feed)

    def take_snapshot(self):
        self.snapshot_button.config(bg = "green")
        self.cam_feed.take_snapshot()

        self.master.after(2000, lambda: self.snapshot_button.config(bg = "white"))

    def open_serial(self):
        if self.stripchart.conn is None :  # Check if Serial Connection Already Established
            port = self.port_entry.get()
            baudrate = self.baudrate_entry.get()

            try :
                self.conn = ArduinoSerial(
                    port = port, baudrate = int(baudrate)
                )

                self.stripchart.start(self.conn) # Start StripChart
                self.stripchart.canvas_widget.grid(row = 0, column = 0)
                self.open_button.config(state = tk.DISABLED)
            except serial.SerialException as serial_error:
                print("Serial Connection Error:", str(serial_error))
                self.conn = None
                self.open_button.config(state = tk.NORMAL)

    def save_data(self):
        if self.stripchart.data_df.empty:
            print("No Data Available")
            return

        self.stripchart.stop()
        self.open_button.config(state = tk.NORMAL)

    def cleanup(self):
        self.stripchart.stop() # Stop StripChart
        self.cam_feed.stop() # Close Camera

class DashboardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot Dashboard")
        # self.geometry("1000x800")
        # self.resizable(False, False)

        self.dashboard = Dashboard(self)

        self.protocol("WM_DELETE_WINDOW", self.on_close) # Cleanup

    def on_close(self):
        """Close Serial Connection on Application Exit."""
        self.dashboard.cleanup() # Cleanup

        # Exit Application
        self.destroy()
        sys.exit()

if __name__ == "__main__":
    DashboardApp().mainloop()