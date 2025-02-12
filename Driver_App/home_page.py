import asyncio
import threading as td

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout

from kivy.uix.spinner import Spinner
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.widget import Widget

from kivy.clock import Clock

from arduinoSerial import *

from bleak import BleakScanner, BleakClient
from bleak.exc import BleakError

# Define UUIDs for  BLE Service and Characteristic
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
        device_layout = AnchorLayout(
            anchor_x = 'left', anchor_y = 'top',
            size_hint = (1, None), # Width : 100% Available
            height = 0.5 * Window.height # 50% of Window Height
        )

        device_grid = GridLayout(
            cols = 1, rows = 2,
            size_hint = (None, None),
            width = 350, height = 150
        )

        self.status_label = Label(
            text = "",
            size_hint = (1, None), # Width : 100% Available
            height = 50
        )

        connect_row = GridLayout(
            cols = 3, rows = 1,
            spacing = 10, # 10 px Between Widgets
            size_hint = (1, None), # Width : 100% Available
            height = 50
        )

        self.device_spinner = Spinner(
            text = "Select Device",
            values = [],
            size_hint = (None, None),
            height = 50, width = 150,
        )
        self.connect_btn = Button(
            text = "Connect",
            disabled = True, # Enable After Scan
            size_hint = (None, None),
            height = 50, width = 100,
        )
        self.disconnect_btn = Button(
            text = "Disconnect",
            disabled = True, # Enable After Connect
            size_hint = (None, None),
            height = 50, width = 100,
        )

        connect_row.add_widget(self.device_spinner)
        connect_row.add_widget(self.connect_btn)
        connect_row.add_widget(self.disconnect_btn)

        device_grid.add_widget(self.status_label)
        device_grid.add_widget(connect_row)

        device_layout.add_widget(device_grid)

        # ------------------------------#
        # Button Section (For Commands) #
        # ------------------------------#
        btn_grid = GridLayout(
            cols = 3, rows = 3, # Tic-Tac-Toe Like Grid
            spacing = 10,
            size_hint = (1, None), # Width : 100% Available
            height = 300  # Adjust for Button Size
        )

        # Movement Command Buttons
        self.btn_up = Button(text = "\u2191", disabled = True, font_name = "DejaVuSans.ttf")
        self.btn_left = Button(text = "\u2190", disabled = True, font_name = "DejaVuSans.ttf")
        self.btn_right = Button(text = "\u2192", disabled = True, font_name = "DejaVuSans.ttf")
        self.btn_down = Button(text = "\u2193", disabled = True, font_name = "DejaVuSans.ttf")

        # Provide Command Attributes to Buttons

        self.btn_up.command = "^" # 122 RPM = 25%
        self.btn_down.command = "v" # 143.5 RPM = 50%
        self.btn_left.command = "<" # 215 RPM = 75%
        self.btn_right.command = ">" # 289 RPM (Max) = 100%

        # Add Widgets to Button Layout
        btn_grid.add_widget(Widget())
        btn_grid.add_widget(self.btn_up)
        btn_grid.add_widget(Widget())

        btn_grid.add_widget(self.btn_left)
        btn_grid.add_widget(Widget())
        btn_grid.add_widget(self.btn_right)

        btn_grid.add_widget(Widget())
        btn_grid.add_widget(self.btn_down)
        btn_grid.add_widget(Widget())

        # -------------------------#
        # Add Sections to the Page #
        # -------------------------#
        self.add_widget(device_layout)
        self.add_widget(btn_grid)

        # BLE Variables
        self.devices = []
        self.client = None
        self.selected_device = None

        # Bind Button Events
        self.connect_btn.bind(on_press = self._connect)
        self.disconnect_btn.bind(on_press = self._disconnect)

        # Bind Movement Button Events (Send Command)
        self.btn_up.bind(on_press = self._send_command)
        self.btn_down.bind(on_press = self._send_command)
        self.btn_left.bind(on_press = self._send_command)
        self.btn_right.bind(on_press = self._send_command)

        # Schedule Scanning for BLE Devices Using Asyncio
        Clock.schedule_once(
            lambda dt: asyncio.run_coroutine_threadsafe(
                self.scan_devices(), self.app.async_loop
            )
        )

    def toggle_mov(self):
        """Toggle Movement Buttons."""
        self.btn_up.disabled = not(self.btn_up.disabled)
        self.btn_down.disabled = not(self.btn_down.disabled)
        self.btn_left.disabled = not(self.btn_left.disabled)
        self.btn_right.disabled = not(self.btn_right.disabled)

    async def scan_devices(self):
        """Scan for Nearby BLE Devices."""
        def update_spinner(dt):
            if self.devices:
                device_names = [device.name for device in self.devices]
                self.device_spinner.values = device_names
                self.status_label.text = f"Found {len(device_names)} Devices."
                self.connect_btn.disabled = False
            else:
                self.status_label.text = "No BLE Devices Found."
            return

        self.status_label.text = "Scan for BLE..." # Update Status Label
        scanned_devices = await BleakScanner.discover()

        self.devices = [device for device in scanned_devices if device.name and "BLE" in device.name] # Filter BLE Devices
        Clock.schedule_once(update_spinner) # Update Spinner Values

    async def connect_device(self):
        selected = [device for device in self.devices if device.name == self.device_spinner.text]
        if self.device_spinner.text == "Select Device": # No Device Selected
            self.status_label.text = "Please Select a Device."
        elif not selected: # Device Not Found
            self.status_label.text = "Selected Device Not Found."
        else:
            self.status_label.text = f"Connecting to {selected[0].name}..."

            # Connect to Selected Device
            self.selected_device = selected[0] # Get Selected Device
            self.client = BleakClient(address_or_ble_device = self.selected_device.address)
            try:
                await self.client.connect()
                if self.client.is_connected: # Connection Successful
                    self.status_label.text = f"Connected to {self.selected_device.name}!"

                    # Update Button States
                    self.connect_btn.disabled = True
                    self.disconnect_btn.disabled = False

                    # Enable Movement Buttons
                    self.toggle_mov()
                else:
                    self.status_label.text = "Connection Failed."
            except BleakError as ble_error:
                self.status_label.text = f"Connection Error: {str(ble_error)}"

    async def disconnect_device(self):
        """Disconnect from BLE Device."""
        if self.client and self.client.is_connected: # Check if Connected
            await self.client.disconnect()
            if self.selected_device:
                self.status_label.text = f"Disconnected from {self.selected_device.name}."
            else:
                self.status_label.text = "Disconnected from Device."

            # Update Button States
            self.connect_btn.disabled = False
            self.disconnect_btn.disabled = True

            # Disable Movement Buttons
            self.toggle_mov()

    async def send_command(self, command):
        """Send Command to Connected BLE Device."""
        if self.client and self.client.is_connected: # Check if Connected
            try:
                await self.client.write_gatt_char(CHARACTERISTIC_UUID, command.encode('utf-8'))
                self.status_label.text = f"Sent Command: {command}"
            except BleakError as ble_error:
                self.status_label.text = f"Error Sending Command: {str(ble_error)}"
        else:
            self.status_label.text = "Not Connected to Device."

    def _connect(self, instance):
        """Schedule Connect Coroutine."""
        asyncio.run_coroutine_threadsafe(self.connect_device(), self.app.async_loop)

    def _disconnect(self, instance):
        """Schedule Disconnect Coroutine."""
        asyncio.run_coroutine_threadsafe(self.disconnect_device(), self.app.async_loop)

    def _send_command(self, instance):
        """Schedule Send Command Coroutine Using Button's Custom Command."""
        command = getattr(instance, 'command', instance.text)
        asyncio.run_coroutine_threadsafe(self.send_command(command), self.app.async_loop)

class TestApp(App):
    def build(self):
        self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        td.Thread(target = self._start_async_loop, daemon = True).start() # Start Event Loop in Separate Thread

        # Configure Window
        Window.size = (1000, 800)
        Window.resizable = False
        self.title = "Arduino Bluetooth Driver"
        self.home_page = HomePageLayout(app = self)
        return self.home_page

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()


if __name__ == "__main__":
    TestApp().run()
