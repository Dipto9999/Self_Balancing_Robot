import asyncio
import threading as td

import datetime as dt

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout

from kivy.uix.image import Image
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.clock import Clock

from kivy_garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg

from arduinoSerial import *
from camera import CameraDisplay
from stripchart import StripChart

class AppLayout(BoxLayout):
    def __init__(self, app, **kwargs):
        super().__init__(orientation = 'vertical', **kwargs)
        self.add_widget(app.dashboard)

class Dashboard(GridLayout):
    def __init__(self, app, **kwargs):
        super().__init__(rows = 1, size_hint = (1, 1), **kwargs)
        self.app = app

        self.record_button = Button(
            text = "Record",
            size_hint = (0.25, 0.25),
            pos_hint = {'center_y': 0.1},
            background_color = "white"
        )
        self.snapshot_button = Button(
            text = "Snapshot",
            size_hint = (0.25, 0.25),
            pos_hint = {'center_y': 0.1},
            background_color = "white"
        )

        # self.cam_feed = Image(size_hint = (1, 1), allow_stretch = True)
        self.cam_feed = CameraDisplay()
        self.strip_chart = StripChart(conn = self.app.conn)

        self.button_layout = BoxLayout(orientation = "horizontal", spacing = 1, padding = (1, 1))
        self.button_layout.add_widget(self.record_button)
        self.button_layout.add_widget(self.snapshot_button)

        self.cam_layout = BoxLayout(orientation = "vertical", spacing = 1, padding = (1, 1))
        self.cam_layout.add_widget(
            Label(text = "Pi-Camera", padding = (1, 1), size_hint = (1, 0.05))
        )
        self.cam_layout.add_widget(self.cam_feed)

        self.top_layout = GridLayout(cols = 2)
        self.top_layout.add_widget(self.button_layout)
        self.top_layout.add_widget(self.cam_layout)

        self.fig_canvas = FigureCanvasKivyAgg(self.strip_chart.fig, size_hint = (1, 1))

        self.pg_layout = GridLayout(
            rows = 2,
            spacing = 5,
            size_hint = (1, 1)
        )
        self.pg_layout.add_widget(self.top_layout)
        self.pg_layout.add_widget(self.fig_canvas)

        self.add_widget(self.pg_layout)

        self.record_button.bind(on_press = self.toggle_record)
        self.snapshot_button.bind(on_press = self.take_snapshot)

        Clock.schedule_interval(self.update_camera, CameraDisplay.SAMPLE_RATE) # Update Camera
        Clock.schedule_interval(self.update_chart, StripChart.SAMPLE_RATE) # Update Plot

    def toggle_record(self, instance):
        # pass

        if self.cam_feed.filename != "": # Recording
            self.cam_feed.stop_recording()
            self.record_button.text = "Start Recording"
            self.record_button.background_color = "white"
        else: # Not Recording
            self.cam_feed.start_recording()
            self.record_button.text = "Stop Recording"
            self.record_button.background_color = "red"

    def take_snapshot(self, instance):
        # pass
        self.cam_feed.take_snapshot()

    def update_camera(self, dt):
        self.cam_feed.update()

    def update_chart(self, dt):
        """Update Strip Chart."""
        self.strip_chart.update() # Update Strip Chart
        self.fig_canvas.draw_idle() # Update Canvas

class DashboardApp(App):
    def build(self) -> AppLayout:
        self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        self.async_thread = td.Thread(target = self._start_async_loop, daemon = True)
        self.async_thread.start() # Start Event Loop in Separate Thread

        # Serial Connection
        try:
            self.conn = ArduinoSerial(port = 'COM3', baudrate = 9600)
        except serial.SerialException as serial_error:
            print("Serial Connection Error:", str(serial_error))
            self.conn = None

        # Configure Window
        Window.size = (1000, 800)
        Window.resizable = False

        self.title = "Robot Dashboard"
        self.dashboard = Dashboard(app = self)

        return AppLayout(app = self)

    def on_stop(self):
        """Close Serial Connection on Application Exit."""
        if self.conn is not None and self.conn.isOpen():
            self.conn.close() # Close Serial Connection When Plot Closed

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.dashboard.strip_chart.save_logs(fig_name)
            self.dashboard.strip_chart.save_fig(fig_name)

        if self.async_loop.is_running():
            self.async_loop.call_soon_threadsafe(self.async_loop.stop)
            self.async_thread.join()

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()
