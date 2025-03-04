import asyncio
import threading

from flask import Flask, request, jsonify, render_template
from bleak import BleakScanner, BleakClient, BleakError

# ---------------------------
# GLOBAL CONFIGURATION
# ---------------------------
# BLE Service & Characteristic from your Arduino code
SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

app = Flask(__name__)

# Globals for BLE
ble_loop = None
ble_client = None
is_connected = False

# Keep track of discovered devices
discovered_devices = []

# ---------------------------
# BACKGROUND EVENT LOOP
# ---------------------------
def run_ble_loop():
    """Run an asyncio event loop for BLE in a background thread."""
    asyncio.set_event_loop(ble_loop)
    ble_loop.run_forever()

def start_background_loop():
    """Initialize and start the BLE event loop in a separate thread."""
    global ble_loop
    ble_loop = asyncio.new_event_loop()
    thread = threading.Thread(target=run_ble_loop, daemon=True)
    thread.start()

def run_async(coro):
    """Helper to schedule an async function on the BLE loop."""
    return asyncio.run_coroutine_threadsafe(coro, ble_loop)

# ---------------------------
# ASYNC BLE OPERATIONS
# ---------------------------
async def scan_for_devices():
    """
    Scan for nearby BLE devices.
    Filter for devices whose name contains 'BLE'.
    """
    global discovered_devices
    discovered_devices = []

    devices = await BleakScanner.discover()
    for d in devices:
        # Only include devices whose name has 'BLE'
        if d.name and "BLE" in d.name:
            discovered_devices.append({"name": d.name, "address": d.address})

    return discovered_devices

async def ble_connect(address):
    """Connect to the BLE device at the given address."""
    global ble_client, is_connected

    # If already connected, do nothing
    if ble_client and ble_client.is_connected:
        is_connected = True
        return True

    ble_client = BleakClient(address)
    try:
        await ble_client.connect()
        is_connected = ble_client.is_connected
        return is_connected
    except BleakError as e:
        print(f"[BLE] Connection Error: {e}")
        is_connected = False
        return False

async def ble_disconnect():
    """Disconnect from the current BLE device."""
    global ble_client, is_connected
    if ble_client and ble_client.is_connected:
        await ble_client.disconnect()
    is_connected = False

async def ble_send_command(cmd):
    """Send a command (e.g., '^', 'v', '<', '>') to the connected BLE device."""
    global ble_client, is_connected

    if not (ble_client and ble_client.is_connected):
        return False, "Not connected to BLE"

    try:
        await ble_client.write_gatt_char(CHARACTERISTIC_UUID, cmd.encode("utf-8"))
        return True, f"Sent command: {cmd}"
    except BleakError as e:
        return False, f"Error sending command: {e}"

# ---------------------------
# FLASK ROUTES
# ---------------------------
@app.route("/")
def index():
    """Serve the main HTML page (templates/index.html)."""
    return render_template("index.html")

@app.route("/scan", methods=["GET"])
def scan():
    """
    Trigger BLE scanning and return a JSON list of discovered devices.
    Each device is { "name": "...", "address": "..." }.
    """
    future = run_async(scan_for_devices())
    devices = future.result(timeout=15)  # Wait up to 15s
    return jsonify(devices)

@app.route("/connect", methods=["POST"])
def connect():
    """
    POST JSON: { "address": "xx:xx:xx:xx:xx:xx" }
    to connect to the chosen device address.
    """
    data = request.get_json()
    if not data or "address" not in data:
        return jsonify({"error": "No address provided"}), 400

    address = data["address"]
    future = run_async(ble_connect(address))
    success = future.result(timeout=10)

    if success:
        return jsonify({"status": "connected"})
    else:
        return jsonify({"status": "failed"}), 400

@app.route("/disconnect", methods=["GET"])
def disconnect():
    """Disconnect from the currently connected BLE device."""
    future = run_async(ble_disconnect())
    future.result(timeout=5)
    return jsonify({"status": "disconnected"})

@app.route("/move", methods=["POST"])
def move():
    """
    POST JSON: { "command": "^" }
    to send a movement command to the connected BLE device.
    """
    data = request.get_json()
    if not data or "command" not in data:
        return jsonify({"error": "No 'command' provided"}), 400

    cmd = data["command"]
    future = run_async(ble_send_command(cmd))
    success, message = future.result(timeout=5)

    if success:
        return jsonify({"status": "ok", "message": message})
    else:
        return jsonify({"status": "error", "message": message}), 400

# ---------------------------
# ENTRY POINT
# ---------------------------
if __name__ == "__main__":
    start_background_loop()
    app.run(host="0.0.0.0", port=5000, debug=True)
