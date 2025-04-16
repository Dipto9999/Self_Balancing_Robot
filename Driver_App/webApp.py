from generateKey import KeyGenerator
import os

from functools import wraps
from datetime import timedelta

import asyncio
import threading as td

from bleak import BleakScanner, BleakClient, BleakError
from flask import Flask, request, jsonify, render_template, redirect, url_for, session

class BluetoothManager:
    # Define UUIDs for BLE Service and Characteristic
    SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
    CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"
    PASSWORD = "EVE" # Authentication Code

    def __init__(self):
        self.devices = []
        self.client = None
        self.is_connected = False

        self.loop = asyncio.new_event_loop()
        self.start_loop()

    def start_loop(self):
        td.Thread(target = self._run_loop, daemon = True).start()

    def _run_loop(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def run_async(self, coro):
        return asyncio.run_coroutine_threadsafe(coro, self.loop)

    async def scan(self):
        self.devices = []
        scanned = await BleakScanner.discover()
        self.devices = [
            {"name" : device.name, "address" : device.address} for device in scanned if device.name and ("WALL-E" in device.name)
        ] # Filter BLE Devices
        return self.devices

    async def connect(self, address):
        """Connect to BLE Device."""
        if self.client and self.client.is_connected:
            self.is_connected = True
            return True
        self.client = BleakClient(address)
        try:
            await self.client.connect()
            await self.client.write_gatt_char(BluetoothManager.CHARACTERISTIC_UUID, BluetoothManager.PASSWORD.encode("utf-8"))
            self.is_connected = self.client.is_connected
            return self.is_connected
        except BleakError:
            self.is_connected = False
            return False

    async def disconnect(self):
        """Disconnect from BLE Device."""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
        self.is_connected = False

    async def send_cmd(self, cmd):
        """Schedule Send Command Coroutine Using Button's Custom Command."""
        if not (self.client and self.client.is_connected):
            return False, "Device Disconnected"

        try:
            await self.client.write_gatt_char(BluetoothManager.CHARACTERISTIC_UUID, cmd.encode("utf-8"))
            return True, f"Sent Command: {cmd}"
        except BleakError as e:
            return False, f"Error Sending Command: {e}"
class RobotDriverApp:
    def __init__(self):
        self.app = Flask("Robot Driver App")
        self.app.secret_key = os.environ.get("FLASK_SECRET_KEY", "fallback_dev_key")

        self.app.config['SESSION_PERMANENT'] = False # Session Expires on Browser Close
        self.app.permanent_session_lifetime = timedelta(minutes = 15) # Inactive Session Timeout

        self.bluetooth_manager = BluetoothManager()
        self._register_routes()

    def login(self, f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            print("Session Auth =", session.get("authenticated"))
            if not session.get("authenticated"):
                # Redirect to Login Page
                return redirect(url_for("auth_handler"))
            # Otherwise, Proceed to Route
            return f(*args, **kwargs)
        return decorated_function

    def _register_routes(self):
        @self.app.route("/login", methods = ["GET", "POST"])
        def auth_handler():
            error = None
            if request.method == "POST":
                if request.form["password"] == BluetoothManager.PASSWORD:
                    session["authenticated"] = True
                    return redirect(url_for("index"))
                else:
                    error = "Access Denied!"
            return render_template("login.html", error = error)

        @self.app.route("/logout", methods = ["POST"])
        def logout():
            future = self.bluetooth_manager.run_async(self.bluetooth_manager.disconnect())
            future.result(timeout = 10) # Wait for 10 Seconds
            session.clear()
            return redirect(url_for("auth_handler"))

        @self.app.route("/")
        @self.login
        def index():
            return render_template("index.html")

        @self.app.route("/scan", methods = ["GET"])
        @self.login
        def scan():
            future = self.bluetooth_manager.run_async(self.bluetooth_manager.scan())
            devices = future.result(timeout = 30) # Wait for 30 Seconds
            return jsonify(devices)

        @self.app.route("/connect", methods = ["POST"])
        @self.login
        def connect():
            data = request.get_json()
            if not data or "deviceAddress" not in data:
                return jsonify({"error": "No Address Provided"}), 400
            future = self.bluetooth_manager.run_async(self.bluetooth_manager.connect(data["deviceAddress"]))
            success = future.result(timeout = 15) # Wait for 15 Seconds
            return (jsonify({"status": "Connected"}) if success
                    else (jsonify({"status": "Failed"}), 400))

        @self.app.route("/disconnect", methods = ["GET"])
        @self.login
        def disconnect():
            future = self.bluetooth_manager.run_async(self.bluetooth_manager.disconnect())
            future.result(timeout = 10) # Wait for 10 Seconds
            return jsonify({"status": "Disconnected"})

        @self.app.route("/command", methods = ["POST"])
        @self.login
        def command():
            data = request.get_json()
            if not data or "command" not in data:
                return jsonify({"error": "No Command Provided"}), 400 # Bad Request
            future = self.bluetooth_manager.run_async(
                self.bluetooth_manager.send_cmd(data["command"])
            )
            success, message = future.result(timeout = 10) # Wait for 10 Seconds

            if success:
                return jsonify({"status": "OK", "msg": message})
            else:
                return jsonify({"status": "Error", "msg": message}), 400 # Bad Request

    def run(self):
        self.app.run(host = "0.0.0.0", port = 5000, debug = False)

app = RobotDriverApp().app  # Exposed for Gunicorn

# if __name__ == "__main__":
#     KeyGenerator()
#     RobotDriverApp().run()
