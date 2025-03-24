import asyncio
import threading

from bleak import BleakScanner, BleakClient, BleakError
from flask import Flask, request, jsonify, render_template


class BLEManager:
    # Define UUIDs for  BLE Service and Characteristic
    SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
    CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

    def __init__(self):
        self.devices = []
        self.client = None
        self.is_connected = False

        self.loop = asyncio.new_event_loop()
        self.start_loop()

    def start_loop(self):
        threading.Thread(target = self._run_loop, daemon = True).start()

    def _run_loop(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def run_async(self, coro):
        return asyncio.run_coroutine_threadsafe(coro, self.loop)

    async def scan_devices(self):
        self.devices = []
        scanned = await BleakScanner.discover()

        self.devices = [
            {"name" : device.name, "address" : device.address} for device in scanned if device.name and ("BLE" in device.name)
        ] # Filter BLE Devices
        return self.devices

    async def connect_device(self, address):
        """Connect to BLE Device."""
        if self.client and self.client.is_connected:
            self.is_connected = True
            return True

        self.client = BleakClient(address)
        try:
            await self.client.connect()
            self.is_connected = self.client.is_connected
            return self.is_connected
        except BleakError:
            self.is_connected = False
            return False

    async def disconnect_device(self):
        """Disconnect from BLE Device."""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
        self.is_connected = False

    async def send_cmd(self, cmd):
        """Schedule Send Command Coroutine Using Button's Custom Command."""
        if not (self.client and self.client.is_connected):
            return False, "Not Connected to BLE"
        try:
            await self.client.write_gatt_char(BLEManager.CHARACTERISTIC_UUID, cmd.encode("utf-8"))
            return True, f"Sent Command: {cmd}"
        except BleakError as e:
            return False, f"Error Sending Command: {e}"

class RobotDriverApp:
    def __init__(self):
        self.app = Flask("Robot Driver App")
        self.ble_manager = BLEManager()
        self._register_routes()

    def _register_routes(self):
        @self.app.route("/")
        def index():
            return render_template("index.html")

        @self.app.route("/scan", methods = ["GET"])
        def scan():
            future = self.ble_manager.run_async(self.ble_manager.scan_devices())
            devices = future.result(timeout = 15) # Wait for 15 Seconds
            return jsonify(devices)

        @self.app.route("/connect", methods = ["POST"])
        def connect():
            data = request.get_json()
            if not data or "deviceAddress" not in data:
                return jsonify({"error": "No Address Provided"}), 400
            future = self.ble_manager.run_async(self.ble_manager.connect_device(data["deviceAddress"]))
            success = future.result(timeout = 10) # Wait for 10 Seconds
            return (jsonify({"status": "Connected"}) if success
                    else (jsonify({"status": "Failed"}), 400))

        @self.app.route("/disconnect", methods = ["GET"])
        def disconnect():
            future = self.ble_manager.run_async(self.ble_manager.disconnect_device())
            future.result(timeout = 10) # Wait for 5 Seconds
            return jsonify({"status": "Disconnected"})

        @self.app.route("/move", methods = ["POST"])
        def move():
            data = request.get_json()
            if not data or "command" not in data:
                return jsonify({"error": "No Command Provided"}), 400 # Bad Request

            future = self.ble_manager.run_async(
                self.ble_manager.send_cmd(data["command"])
            )
            success, message = future.result(timeout = 10) # Wait for 5 Seconds

            if success:
                return jsonify({"status": "OK", "msg": message})
            else:
                return jsonify({"status": "Error", "msg": message}), 400 # Bad Request

    def run(self):
        self.app.run(host = "0.0.0.0", port = 5000, debug = False)

if __name__ == "__main__":
    RobotDriverApp().run()
