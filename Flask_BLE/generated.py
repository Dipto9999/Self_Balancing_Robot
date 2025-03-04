import asyncio
import threading

from flask import Flask, request, jsonify
from bleak import BleakClient, BleakError

# ------------------------------------------------------------------------
# CONFIGURATION
# ------------------------------------------------------------------------
# Your Arduino BLE device address (or UUID) goes here.
# Replace with your actual BLE address (or read from an environment variable).
ARDUINO_BLE_ADDRESS = "XX:XX:XX:XX:XX:XX"

# The UUIDs from your Arduino firmware/home_page.py
SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

# Create the Flask application
app = Flask(__name__)

# ------------------------------------------------------------------------
# GLOBALS FOR BLE
# ------------------------------------------------------------------------
ble_loop = None       # The main asyncio event loop for BLE
ble_client = None     # BleakClient instance
is_connected = False  # Track connection status

# ------------------------------------------------------------------------
# 1) BACKGROUND THREAD & EVENT LOOP
# ------------------------------------------------------------------------
def run_ble_loop():
    """Continuously runs the BLE event loop in a background thread."""
    asyncio.set_event_loop(ble_loop)
    ble_loop.run_forever()

def start_background_loop():
    """Starts the global BLE event loop in a separate daemon thread."""
    global ble_loop
    ble_loop = asyncio.new_event_loop()
    thread = threading.Thread(target=run_ble_loop, daemon=True)
    thread.start()

def run_async(coro):
    """Helper to schedule an async coroutine in the BLE event loop."""
    return asyncio.run_coroutine_threadsafe(coro, ble_loop)

# ------------------------------------------------------------------------
# 2) ASYNC FUNCTIONS
# ------------------------------------------------------------------------
async def ble_connect():
    """Async routine to connect to the Arduino BLE."""
    global ble_client, is_connected

    if ble_client and ble_client.is_connected:
        # Already connected
        is_connected = True
        return True

    # Create a new BleakClient for the known address
    ble_client = BleakClient(ARDUINO_BLE_ADDRESS)
    try:
        await ble_client.connect()
        is_connected = ble_client.is_connected
        return is_connected
    except BleakError as e:
        print(f"[BLE] Connection Error: {e}")
        is_connected = False
        return False

async def ble_disconnect():
    """Async routine to disconnect from the Arduino BLE."""
    global ble_client, is_connected
    if ble_client and ble_client.is_connected:
        await ble_client.disconnect()
        is_connected = False

async def ble_send_command(cmd):
    """Async routine to send a command (e.g., '^', 'v', '<', '>') to Arduino."""
    if not (ble_client and ble_client.is_connected):
        return False, "Not connected to BLE"
    try:
        await ble_client.write_gatt_char(CHARACTERISTIC_UUID, cmd.encode("utf-8"))
        return True, f"Sent command: {cmd}"
    except BleakError as e:
        return False, f"Error sending command: {e}"

# ------------------------------------------------------------------------
# 3) FLASK ROUTES
# ------------------------------------------------------------------------
@app.route("/")
def index():
    return "<h2>Flask Driver App</h2>" \
           "<p>Use /connect to connect, /move to send commands.</p>"

@app.route("/connect", methods=["GET"])
def connect():
    """Route to connect to the Arduino BLE device."""
    future = run_async(ble_connect())
    result = future.result(timeout=10)  # Wait up to 10s for connection
    if result:
        return jsonify({"status": "connected"})
    else:
        return jsonify({"status": "failed"}), 400

@app.route("/move", methods=["POST"])
def move():
    """
    POST JSON: { "command": "^" }
    to send '^', 'v', '<', or '>' to the BLE device.
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

@app.route("/disconnect", methods=["GET"])
def disconnect():
    """Route to disconnect from Arduino BLE device."""
    future = run_async(ble_disconnect())
    future.result(timeout=5)  # Wait up to 5s
    return jsonify({"status": "disconnected"})

# ------------------------------------------------------------------------
# 4) MAIN ENTRY POINT
# ------------------------------------------------------------------------
if __name__ == "__main__":
    # Start the BLE event loop in the background *before* running the Flask server
    start_background_loop()

    # Run Flask on 0.0.0.0 so it's accessible on the local network.
    # Then from your phone, go to http://<PC_IP>:5000
    app.run(host="0.0.0.0", port=5000, debug=True)
