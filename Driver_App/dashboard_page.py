import datetime as dt
import serial

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout

from kivy.uix.image import Image
from kivy.uix.widget import Widget
from kivy.uix.label import Label
from kivy.clock import Clock

from kivy_garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg

from arduinoSerial import ArduinoSerial
from camera import CameraDisplay
from stripchart import StripChart

class DashboardPageLayout(GridLayout):
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

class TestApp(App):
    def build(self):
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