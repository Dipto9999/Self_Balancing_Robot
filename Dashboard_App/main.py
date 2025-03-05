import asyncio
import threading as td

import datetime as dt

from kivy.app import App
from kivy.core.window import Window
from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout

# from kivy.uix.image import Image
from kivy.uix.widget import Widget
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

        # self.cam_feed = Image(size_hint = (1, 1), allow_stretch = True)
        self.cam_feed = CameraDisplay()
        self.strip_chart = StripChart(conn = self.app.conn)

        Clock.schedule_interval(self.update_chart, StripChart.SAMPLE_RATE) # Update Plot
        Clock.schedule_interval(self.update_camera, CameraDisplay.SAMPLE_RATE) # Update Camera

        self.cam_layout = BoxLayout(orientation = "vertical", spacing = 1, padding = (1, 1))
        self.cam_layout.add_widget(
            Label(text = "Pi-Camera", padding = (1, 1), size_hint = (1, 0.05))
        )
        self.cam_layout.add_widget(self.cam_feed)

        self.top_layout = GridLayout(cols = 2)
        self.top_layout.add_widget(Widget())
        self.top_layout.add_widget(self.cam_layout)

        self.fig_canvas = FigureCanvasKivyAgg(self.strip_chart.fig, size_hint = (1, 1))

        self.pg_layout = GridLayout(
            rows = 2,
            spacing = 5,
            size_hint = (1, 1)
        )
        self.pg_layout.add_widget(self.top_layout)
        # self.pg_layout.add_widget(self.cam_layout)
        self.pg_layout.add_widget(self.fig_canvas)

        self.add_widget(self.pg_layout)

    def update_chart(self, dt):
        """Update Strip Chart."""
        self.strip_chart.update() # Update Strip Chart
        self.fig_canvas.draw_idle() # Update Canvas

    def update_camera(self, dt):
        self.cam_feed.update()

class DashboardApp(App):
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

        self.title = "Robot Dashboard"
        self.dashboard = Dashboard(app = self)


        return AppLayout(app = self)

    def on_stop(self):
        """Close Serial Connection on Application Exit."""
        if self.conn is not None and self.conn.isOpen():
            self.conn.close() # Close Serial Connection When Plot Closedconn.close() # Close Serial Connection When Plot Closed

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.dashboard.strip_chart.save_logs(fig_name)
            self.dashboard.strip_chart.save_fig(fig_name)
        self.dashboard.cam_feed.stop()

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

if __name__ == "__main__":
    app = DashboardApp()
    app.run()
