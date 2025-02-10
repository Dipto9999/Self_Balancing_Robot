import asyncio
import threading as td

from kivy.app import App

from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout

from kivy.uix.spinner import Spinner
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.widget import Widget

from kivy.clock import Clock
from bleak import BleakClient, BleakScanner

# Define UUIDs
SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

class HomePageLayout(BoxLayout):
    def __init__(self, app, **kwargs):
        super().__init__(**kwargs)
        self.app = app

        self.orientation = "vertical"
        self.spacing = 10
        self.padding = 10

        # ------------------------------#
        # Device Section (Anchored Top) #
        # ------------------------------#

        self.device_section = AnchorLayout(
            anchor_x = 'left',
            anchor_y = 'top',
            size_hint = (1, 1)
        )

        device_layout = GridLayout(
            cols = 1, rows = 2,
            size_hint = (None, None),
            width = 350, height = 150
        )

        self.status_label = Label(
            text = "Scanning for BLE...",
            size_hint = (1, None),
            height = 50
        )

        connect_row = GridLayout(
            cols = 3, rows = 1,
            spacing = 10,
            size_hint = (1, None),
            height = 50
        )

        self.device_spinner = Spinner(
            text = "Select Device",
            size_hint = (None, None),
            height = 50, width = 150,
        )
        self.connect_btn = Button(
            text = "Connect",
            disabled = True,
            size_hint = (None, None),
            height = 50, width = 100,
        )
        self.disconnect_btn = Button(
            text = "Disconnect",
            disabled = True,
            size_hint = (None, None),
            height = 50, width = 100,
        )

        connect_row.add_widget(self.device_spinner)
        connect_row.add_widget(self.connect_btn)
        connect_row.add_widget(self.disconnect_btn)

        device_layout.add_widget(self.status_label)
        device_layout.add_widget(connect_row)

        self.device_section.add_widget(device_layout)

        # ------------------------------#
        # Button Section (For Commands) #
        # ------------------------------#
        self.btn_layout = GridLayout(
            cols = 3,
            rows = 3,
            spacing = 10,
            size_hint_y = None,
            height = 300 # Adjust for Button Size
        )

        # Top Row
        self.btn_layout.add_widget(Widget())
        self.btn_layout.add_widget(Button(text = "\u2191", disabled = True, font_name = "DejaVuSans.ttf"))
        self.btn_layout.add_widget(Widget())

        # Middle Row
        self.btn_layout.add_widget(Button(text = "\u2190", disabled = True, font_name = "DejaVuSans.ttf"))
        self.btn_layout.add_widget(Widget())
        self.btn_layout.add_widget(Button(text = "\u2192", disabled = True, font_name = "DejaVuSans.ttf"))

        # Bottom Row
        self.btn_layout.add_widget(Widget())
        self.btn_layout.add_widget(Button(text = "\u2193", disabled = True, font_name = "DejaVuSans.ttf"))
        self.btn_layout.add_widget(Widget())

        # -------------------------#
        # Add Sections to the Page #
        # -------------------------#

        self.add_widget(self.device_section)
        self.add_widget(self.btn_layout)

        self.device_section.padding = (0, 10, 10, 0)

        # BLE Variables
        self.devices = []
        self.client = None
        self.selected_device = None

        # Bind Button Events
        self.connect_btn.bind(on_press = self._connect)
        self.disconnect_btn.bind(on_press = self._disconnect)

        for child in self.btn_layout.children:
            if isinstance(child, Button):
                child.bind(on_press = self._send_command)

        # Schedule Scanning for BLE Devices Using Asyncio
        Clock.schedule_once(lambda dt: asyncio.run_coroutine_threadsafe(self.scan_devices(dt), self.app.async_loop))

    async def scan_devices(self, *args):
        """Scan for Nearby BLE Devices."""
        def _update_spinner(dt):
            self.device_spinner.values = self.devices
            self.status_label.text = f"Found {len(self.devices)} Devices."
            self.connect_btn.disabled = False

        self.status_label.text = "Scanning for BLE Devices..."
        scan = await BleakScanner.discover()  # Scan for BLE devices
        self.devices = [device.name or device.address for device in scan if str(device.name).find("BLE") != -1]

        Clock.schedule_once(_update_spinner)

    async def connect_device(self, *args):
        """Connect to Selected BLE Device."""
        if self.device_spinner.text == "Select Device":
            self.status_label.text = "Please Select Device."

        self.selected_device = next(
            device for device in self.devices if device.name == self.device_spinner.text
        )
        self.status_label.text = f"Connecting to {self.selected_device.name}..."

        self.client = BleakClient(self.selected_device)  # Create a BLE Client
        await self.client.connect()  # Connect to BLE Device
        self.status_label.text = f"Connected to {self.selected_device.name}!"

        self.connect_btn.disabled = True
        self.disconnect_btn.disabled = False

    async def disconnect_device(self, *args):
        """Disconnect from BLE Device."""
        if self.client and self.client.is_connected:
            await self.client.disconnect()

            if self.selected_device:
                self.status_label.text = f"Disconnected from {self.selected_device.name}."
            else:
                self.status_label.text = "Disconnected from Device."
            self.connect_btn.disabled = False
            self.disconnect_btn.disabled = True

    async def send_command(self, command):
        """Send Command to Connected BLE Device."""
        if self.client and self.client.is_connected:
            await self.client.write_gatt_char(CHARACTERISTIC_UUID, command.encode())
            self.status_label.text = f"Sent Command: {command}"

    def _connect(self, instance):
        """Schedule Coroutine connect_device."""
        asyncio.run_coroutine_threadsafe(self.connect_device(), self.app.async_loop)

    def _disconnect(self, instance):
        """Schedule Coroutine disconnect_device."""
        asyncio.run_coroutine_threadsafe(self.disconnect_device(), self.app.async_loop)

    def _send_command(self, instance):
        """Schedule Coroutine send_command."""
        command = instance.text
        asyncio.run_coroutine_threadsafe(self.send_command(command), self.app.async_loop)

class TestApp(App):
    def build(self):
        self.title = "Arduino Bluetooth Driver"
        # Create an asyncio event loop and store it as an attribute
        self.async_loop = asyncio.new_event_loop()

        # Start Asyncio Event Loop in Separate Thread
        td.Thread(target = self._start_async_loop, daemon = True).start()
        return HomePageLayout(app = self)

    def _start_async_loop(self):
        """Start the Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

if __name__ == "__main__":
    TestApp().run()