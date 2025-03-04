import asyncio
import threading as td

import os
import shutil
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

        self.convert_video()

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

    def convert_video(self):
        if self.video_name == "":
            return

        video_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")

        input_file = os.path.join(video_dir, f"{self.video_name}.h264")
        temp_file = os.path.join(video_dir, "temp.h264")
        mp4_file = os.path.join(video_dir, f"{self.video_name}.mp4")

        shutil.copy2(input_file, temp_file)
        os.remove(input_file)

        # Flip Video Vertically + Horizontally and Copy Audio
        command = [
            "ffmpeg", # Command
            "-i", temp_file, # Input File
            "-vf", "vflip,hflip", # Vertical and Horizontal Flip
            "-c:a", "copy", # Copy Audio
            mp4_file
        ]

        try:
            subprocess.Popen(
                command,
                stdout = subprocess.DEVNULL,
                stderr = subprocess.DEVNULL,
                start_new_session = True
            )
            print("Detached ffmpeg process started.")
        except Exception as e:
            print("Error Starting Detached Conversion:", e)

if __name__ == "__main__":
    app = DriverApp()
    app.run()
