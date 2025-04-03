import asyncio
import threading

from bleak import BleakScanner, BleakClient

import tkinter as tk
from tkinter import scrolledtext, ttk

class BLECommandApp:
    # Define UUIDs for BLE Service and Characteristic
    SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
    CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

    def __init__(self, root):
        self.root = root
        self.root.title("BLE-B17 Command App")

        self.client = None
        self.connected = False
        self.loop = None
        self.task = None

        self.config()

        # Setup Event Loop in Separate Thread
        self.thread = threading.Thread(target = self.start_loop, daemon = True)
        self.thread.start()

    def config(self):
        # Connection Controls Frame
        self.conn_frame = ttk.LabelFrame(self.root, text = "Connection")

        self.connect_btn = ttk.Button(
            self.conn_frame,
            text = "Connect to BLE-B17",
            command = self.toggle_conn
        )
        self.status_frame = ttk.Frame(self.conn_frame)
        self.status_text = ttk.Label(self.status_frame, text = "Status:")
        self.status_label = ttk.Label(self.status_frame, text = "Disconnected", foreground = "red")
        self.deviceinfo_label = ttk.Label(self.conn_frame, text = "Not Connected")

        # Layout
        self.conn_frame.pack(fill = tk.X, padx = 10, pady = 10)
        self.connect_btn.pack(fill = tk.X, padx = 10, pady = 10)
        self.status_frame.pack(fill=tk.X, padx=10, pady=(0, 10))
        self.status_text.pack(side = tk.LEFT, padx = (0, 5))
        self.status_label.pack(side = tk.LEFT)
        self.deviceinfo_label.pack(fill = tk.X, padx = 10, pady = (0, 10))


        self.log_frame = ttk.LabelFrame(self.root, text = "Received Data")
        self.log_text = scrolledtext.ScrolledText(
            self.log_frame,
            wrap = tk.WORD,
            height = 10,
            state = tk.DISABLED
        )

        self.log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.log_text.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.cmd_frame = ttk.LabelFrame(self.root, text = "Send CMD")
        self.cmd_entry = ttk.Entry(self.cmd_frame)

        self.cmd_frame.pack(fill = tk.X, padx = 10, pady = 10)
        self.cmd_entry.pack(fill = tk.X, padx = 10, pady = 10)

        self.btn_frame = ttk.Frame(self.cmd_frame)
        self.send_btn = ttk.Button(
            self.btn_frame, text = "Send", command = self.send_cmd, state = tk.DISABLED
        )
        self.clear_btn = ttk.Button(self.btn_frame, text = "Clear Log", command = self.clear_log)

        self.btn_frame.pack(fill = tk.X, padx = 10, pady = (0, 10))
        self.send_btn.pack(side=tk.LEFT, padx=(0, 5))
        self.clear_btn.pack(side=tk.LEFT)

        # Bind Enter Key to Send Command
        self.cmd_entry.bind("<Return>", lambda event: self.send_cmd())

    def start_loop(self):
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def update_log(self, msg, tag = ""):
        self.log_text.config(state = tk.NORMAL)
        self.log_text.insert(tk.END, f"{msg}\n", tag)
        self.log_text.see(tk.END)
        self.log_text.config(state = tk.DISABLED)

    def clear_log(self):
        self.log_text.config(state = tk.NORMAL)
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state = tk.DISABLED)

    def toggle_conn(self):
        if not self.loop:
            return

        # Connect/Disconnect
        if not self.connected:
            self.connect_btn.config(state = tk.DISABLED)
            asyncio.run_coroutine_threadsafe(self.connect(), self.loop)
        else:
            asyncio.run_coroutine_threadsafe(self.disconnect(), self.loop)

    def send_cmd(self):
        cmd = self.cmd_entry.get().strip()
        if (not cmd) or (not self.connected) or not (self.loop):
            return

        asyncio.run_coroutine_threadsafe(self.write(cmd), self.loop)
        self.cmd_entry.delete(0, tk.END)

    def display_data(self, data):
        try:
            text = data.decode("utf-8")
        except Exception:
            text = str(data)

        self.root.after(0, lambda: self.update_log(f"Received: {text}"))

    async def connect(self):
        self.root.after(0, lambda: self.update_log("Scanning for BLE-B17..."))

        try:
            scanned = await BleakScanner.discover()

            devices = [
                {"name" : device.name, "address" : device.address} for device in scanned if device.name and ("BLE-B17" in device.name)
            ] # Filter BLE Devices

            if len(devices) == 0:
                self.root.after(0, lambda: self.update_log("No BLE-B17 Device Found.", "error"))
                self.root.after(0, lambda: self.connect_btn.config(state = tk.NORMAL))
                return

            self.root.after(0, lambda: self.update_log(f"Connecting to {devices[0]['name']} @Address [{devices[0]['address']}]"))

            client = BleakClient(devices[0]['address'])
            await client.connect()

            if client.is_connected:
                self.client = client
                self.connected = True

                # Start notifications
                await client.start_notify(BLECommandApp.CHARACTERISTIC_UUID, self.display_data)

                self.root.after(0, lambda: self.display_connection(devices[0]))
            else:
                self.root.after(0, lambda: self.update_log("Connection Failed.", "error"))
                self.root.after(0, lambda: self.connect_btn.config(state=tk.NORMAL))

        except Exception as e:
            self.root.after(0, lambda: self.update_log(f"Error: {str(e)}", "error"))
            self.root.after(0, lambda: self.connect_btn.config(state = tk.NORMAL))

    def display_connection(self, device):
        self.status_label.config(text = "Connected", foreground = "green")
        self.deviceinfo_label.config(text = f"Connected to: {device['name']} ({device['address']})")
        self.connect_btn.config(text = "Disconnect", state = tk.NORMAL)
        self.send_btn.config(state = tk.NORMAL)
        self.update_log("Connected!")

    def display_disconnection(self):
        self.status_label.config(text = "Disconnected", foreground = "red")
        self.deviceinfo_label.config(text = "No Device Connected")
        self.connect_btn.config(text = "Connect to BLE-B17", state=tk.NORMAL)
        self.send_btn.config(state = tk.DISABLED)

    async def disconnect(self):
        if self.client and self.client.is_connected:
            try:
                await self.client.stop_notify(BLECommandApp.CHARACTERISTIC_UUID)
                await self.client.disconnect()
            except Exception as e:
                self.root.after(0, lambda: self.update_log(f"Error Disconnecting: {str(e)}", "error"))

        self.client = None
        self.connected = False
        self.root.after(0, self.display_disconnection)
        self.root.after(0, lambda: self.update_log("Disconnected."))

    async def write(self, cmd):
        if not self.client or not self.client.is_connected:
            self.root.after(0, lambda: self.update_log("Not Connected to Device.", "error"))
            return

        try:
            self.root.after(0, lambda: self.update_log(f"Sending: {cmd}"))
            await self.client.write_gatt_char(BLECommandApp.CHARACTERISTIC_UUID, cmd.encode("utf-8"))
        except Exception as e:
            self.root.after(0, lambda: self.update_log(f"Error Sending CMD: {str(e)}", "error"))

    def on_close(self):
        if self.connected:
            asyncio.run_coroutine_threadsafe(self.disconnect(), self.loop)
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)
        if self.thread:
            self.thread.join(timeout = 1.0)

        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = BLECommandApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()