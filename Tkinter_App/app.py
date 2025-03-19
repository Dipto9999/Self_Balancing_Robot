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
        super().__init__(master)

        self.master = master

        self.dashboard_frame = tk.Frame(self.master, bg = '#000000')

        self.camera_frame = tk.Frame(self.dashboard_frame, bg = '#141654')
        self.serial_frame = tk.Frame(self.dashboard_frame, bg = '#787882')
        self.stripchart_frame = tk.Frame(self.dashboard_frame, bg = '#48484d')

        ############################
        ### Camera Frame Widgets ###
        ############################

        self.button_frame = tk.Frame(self.camera_frame, bg = '#787882')

        self.record_button = tk.Button(
            self.button_frame,
            text = "Start Recording", width = 15,
            command = self.toggle_record
        )
        self.snapshot_button = tk.Button(
            self.button_frame,
            text = "Snapshot", width = 15,
            command = lambda: self.cam_feed.take_snapshot()
        )

        self.cam_feed = CameraDisplay(self.camera_frame) # Camera Feed

        self.record_button.pack(side = tk.LEFT, padx = 5, pady = 5)
        self.snapshot_button.pack(side = tk.RIGHT, padx = 5, pady = 5)

        self.camera_frame.pack(side = tk.TOP, fill = tk.X)
        self.button_frame.pack(side = tk.BOTTOM, fill = tk.X)

        ############################
        ### Serial Frame Widgets ###
        ############################

        self.port_label = tk.Label(self.serial_frame, text = "Serial Line : ", bg = '#787882')
        self.port_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')

        self.baudrate_label = tk.Label(self.serial_frame, text = "Speed : ", bg = '#787882')
        self.baudrate_entry = tk.Entry(self.serial_frame, bg = '#6e9eeb')

        self.open_button = tk.Button(
            self.serial_frame, text = "Open", command = self.open_serial, bg = '#6e9eeb',
        )

        self.port_label.grid(
            row = 0, column = 0,
            rowspan = 1, columnspan = 1,
            padx = 10, pady = 10, sticky = 'w'
        )
        self.port_entry.grid(
            row = 0, column = 1,
            rowspan = 1, columnspan = 1,
            padx = 10, pady = 10
        )

        self.baudrate_label.grid(
            row = 1, column = 0,
            rowspan = 1, columnspan = 1,
            padx = 10, pady = 10, sticky = 'w'
        )
        self.baudrate_entry.grid(
            row = 1, column = 1,
            rowspan = 1, columnspan = 1,
            padx = 10, pady = 10
        )

        self.open_button.grid(
            row = 2, column = 1,
            rowspan = 1, columnspan = 1,
            padx = 10, pady = 10,
            sticky = 'e'
        )

        ################################
        ### StripChart Frame Widgets ###
        ################################

        self.stripchart = StripChart(self.stripchart_frame)

        self.stripchart.canvas_widget.grid(row = 0, column = 0)

        # Position Widgets

        self.camera_frame.grid(row = 0, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.serial_frame.grid(row = 1, column = 0, padx = 10, pady = 10, sticky = tk.NSEW)
        self.stripchart_frame.grid(row = 0, column = 1, rowspan = 2, padx = 10, pady = 10, sticky = tk.NSEW)

        self.dashboard_frame.pack(fill = tk.BOTH, expand = True)

        self.after(500, self.update_feed)

    def toggle_record(self):
        if self.cam_feed.filename: # Currently Recording
            self.cam_feed.stop_recording()
            self.record_button.config(text = "Start", bg = "SystemButtonFace")
        else: # Not Recording
            self.cam_feed.start_recording()
            self.record_button.config(text = "Stop", bg = "red")

    def update_feed(self):
        self.cam_feed.update()
        self.after(int(CameraDisplay.SAMPLE_RATE * 10E3), self.update_feed)

    def open_serial(self):
        if self.conn is None :  # Check if Serial Connection Already Established
            port = self.port_entry.get()
            baudrate = self.baudrate_entry.get()

            try :
                self.conn = ArduinoSerial(
                    port = port, baudrate = int(baudrate)
                )

                self.stripchart.start(self.conn) # Start StripChart
                self.stripchart.canvas_widget.grid(row = 0, column = 0)
            except serial.SerialException as serial_error:
                print("Serial Connection Error:", str(serial_error))
                self.conn = None

    def cleanup(self):
        if self.conn and self.conn.isOpen():
            self.stripchart.stop() # Stop StripChart

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.stripchart.save_logs(fig_name)
            self.stripchart.save_fig(fig_name)

        self.cam_feed.stop() # Close Camera

class DashboardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot Dashboard")
        self.geometry("1000x800")
        self.resizable(False, False)

        self.dashboard = Dashboard(self)

        self.protocol("WM_DELETE_WINDOW", self.on_close) # Cleanup

    def on_close(self):
        """Close Serial Connection on Application Exit."""
        self.dashboard.cleanup() # Cleanup

        # Exit Application
        self.destroy()
        sys.exit()