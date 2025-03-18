import tkinter as tk
from tkinter import Button, Frame
import datetime as dt
import serial

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

from camera import CameraDisplay
from stripchart import StripChart
from arduinoSerial import ArduinoSerial

class Dashboard(Frame):
    def __init__(self, parent):
        super().__init__(parent)

        # Serial Connection
        try:
            self.conn = ArduinoSerial(port = 'COM3', baudrate = 9600)
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None

        # Camera
        self.cam_feed = CameraDisplay(self)
        self.cam_feed.pack(side = tk.LEFT, padx = 10, pady = 10)

        # Button Frame
        self.button_frame = Frame(self)
        self.button_frame.pack(side = tk.TOP, fill = tk.X)

        self.record_button = Button(
            self.button_frame,
            text = "Start Recording", width = 15,
            command = self.toggle_record
        )
        self.record_button.pack(side = tk.LEFT, padx = 5, pady = 5)

        self.snapshot_button = Button(
            self.button_frame,
            text = "Snapshot", width = 15,
            command = self.cam_feed.take_snapshot
        )
        self.snapshot_button.pack(side = tk.LEFT, padx = 5, pady = 5)

        # Strip Chart
        self.strip_chart = StripChart(conn = self.conn)
        self.fig_canvas = FigureCanvasTkAgg(self.strip_chart.fig, master = self)
        self.fig_canvas.get_tk_widget().pack(
            side = tk.RIGHT,
            padx = 10, pady = 10,
            fill = tk.BOTH, expand = True
        )

        self.update_chart()
        self.update_feed()

    def toggle_record(self):
        if self.cam_feed.filename: # Currently Recording
            self.cam_feed.stop_recording()
            self.record_button.config(text = "Start", bg = "SystemButtonFace")
        else: # Not Recording
            self.cam_feed.start_recording()
            self.record_button.config(text = "Stop", bg = "red")

    def update_chart(self):
        self.strip_chart.update()
        self.fig_canvas.draw_idle() # Redraw Canvas
        # Schedule the next update
        self.after(int(StripChart.SAMPLE_RATE), self.update_chart)

    def update_feed(self):
        self.cam_feed.update()
        self.after(int(CameraDisplay.SAMPLE_RATE), self.update_feed)

    def cleanup(self):
        if self.conn and self.conn.isOpen():
            self.conn.close() # Close Serial Connection When Plot Closed

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.strip_chart.save_logs(fig_name)
            self.strip_chart.save_fig(fig_name)

        self.cam_feed.stop() # Close Camera

class DashboardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot Dashboard")
        self.geometry("1000x800")
        self.resizable(False, False)

        self.dashboard = Dashboard(self)
        self.dashboard.pack(fill = tk.BOTH, expand = True)

        self.protocol("WM_DELETE_WINDOW", self.on_close) # Cleanup

    def on_close(self):
        """Close Serial Connection on Application Exit."""
        self.dashboard.cleanup() # Cleanup
        self.destroy() # Quit Application