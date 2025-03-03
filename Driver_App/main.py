import asyncio
import multiprocessing as mp
import threading as td

import os
import subprocess
import time

import datetime as dt

from kivy.app import App
from kivy.core.window import Window
from kivy.uix.tabbedpanel import TabbedPanel, TabbedPanelItem

from arduinoSerial import *
from home_page import HomePageLayout
from dashboard_page import DashboardPageLayout

class AppLayout(TabbedPanel):
    def __init__(self, app, **kwargs):
        super().__init__(**kwargs)

        # Home Tab
        home_tab = TabbedPanelItem(text = 'Home')
        home_tab.add_widget(app.home_page)
        self.add_widget(home_tab)

        # Dashboard Tab
        dashboard_tab = TabbedPanelItem(text = 'Dashboard')
        dashboard_tab.add_widget(app.dashboard_page)
        self.add_widget(dashboard_tab)

        self.default_tab = home_tab

class DriverApp(App):
    def build(self) -> AppLayout:
        self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        td.Thread(target = self._start_async_loop, daemon = True).start() # Start Event Loop in Separate Thread

        # Serial Connection
        try:
            self.conn = ArduinoSerial(port = 'COM3', baudrate = 9600)
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None

        # Configure Window
        Window.size = (1000, 800)
        Window.resizable = False

        self.title = "Robot Driver App"
        self.home_page = HomePageLayout(app = self)
        self.dashboard_page = DashboardPageLayout(app = self)

        self.video_name = ""

        return AppLayout(app = self)

    def on_stop(self):
        """Close Serial Connection on Application Exit."""
        if self.conn is not None and self.conn.isOpen():
            self.conn.close() # Close Serial Connection When Plot Closedconn.close() # Close Serial Connection When Plot Closed

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.dashboard_page.strip_chart.save_logs(fig_name)
            self.dashboard_page.strip_chart.save_fig(fig_name)
        self.video_name = self.dashboard_page.cam_feed.stop()

        mp.Process(target = self.convert_video, daemon = False).start()

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

    def convert_video(self):
        if self.video_name == "":
            return

        input_file = f"{self.video_name}.h264"
        video_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")
        input_file = os.path.join(video_dir, input_file)

        # Output File Paths
        mp4_file = os.path.join(video_dir, f"{self.video_name}.mp4")
        temp_file = os.path.join(video_dir, "converted.mp4")

        # Flip Video Vertically + Horizontally and Copy Audio
        command = [
            "ffmpeg", # Command
            "-i", input_file, # Input File
            "-vf", "vflip,hflip", # Vertical and Horizontal Flip
            "-c:a", "copy", # Copy Audio
            temp_file
        ]

        try:
            result = subprocess.run(command, check = True, capture_output = True, text = True) # Run Command
            print("ffmpeg Output:", result.stdout)
            os.replace(temp_file, mp4_file)  # Replace the Temporary File with the MP4 File
            # os.remove(input_file) # Remove the original H264 File
            print(f"Conversion Successful. Video Saved as {mp4_file}")
        except subprocess.CalledProcessError as e:
            print("ffmpeg Conversion Failed:", e)
        except OSError as e:
            print("File Operation Failed:", e)

if __name__ == "__main__":
    app = DriverApp()
    app.run()
