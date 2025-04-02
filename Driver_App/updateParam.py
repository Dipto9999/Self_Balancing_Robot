import asyncio
import threading
import tkinter as tk
from tkinter import scrolledtext, ttk
from bleak import BleakScanner, BleakClient

class BLEControlApp:
    def __init__(self, root):
        self.root = root
        self.root.title("BLE-B17 Device Controller")
        self.root.geometry("600x500")
        self.root.resizable(True, True)

        self.client = None
        self.connected = False
        self.loop = None
        self.task = None

        self.setup_ui()

        # Set up event loop in a separate thread
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self.start_loop, daemon=True)
        self.thread.start()

    def setup_ui(self):
        # Frame for connection controls
        connection_frame = ttk.LabelFrame(self.root, text="Connection")
        connection_frame.pack(fill=tk.X, padx=10, pady=10)

        # Connect button
        self.connect_button = ttk.Button(connection_frame, text="Connect to BLE-B17", command=self.toggle_connection)
        self.connect_button.pack(fill=tk.X, padx=10, pady=10)

        # Status indicator
        status_frame = ttk.Frame(connection_frame)
        status_frame.pack(fill=tk.X, padx=10, pady=(0, 10))

        ttk.Label(status_frame, text="Status:").pack(side=tk.LEFT, padx=(0, 5))
        self.status_label = ttk.Label(status_frame, text="Disconnected", foreground="red")
        self.status_label.pack(side=tk.LEFT)

        # Device information
        self.device_info_label = ttk.Label(connection_frame, text="No device connected")
        self.device_info_label.pack(fill=tk.X, padx=10, pady=(0, 10))

        # Output log
        log_frame = ttk.LabelFrame(self.root, text="Received Data")
        log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.log_text = scrolledtext.ScrolledText(log_frame, wrap=tk.WORD, height=10)
        self.log_text.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.log_text.config(state=tk.DISABLED)

        # Command input
        command_frame = ttk.LabelFrame(self.root, text="Send Command")
        command_frame.pack(fill=tk.X, padx=10, pady=10)

        self.command_entry = ttk.Entry(command_frame)
        self.command_entry.pack(fill=tk.X, padx=10, pady=10)

        button_frame = ttk.Frame(command_frame)
        button_frame.pack(fill=tk.X, padx=10, pady=(0, 10))

        self.send_button = ttk.Button(button_frame, text="Send", command=self.send_command, state=tk.DISABLED)
        self.send_button.pack(side=tk.LEFT, padx=(0, 5))

        self.clear_button = ttk.Button(button_frame, text="Clear Log", command=self.clear_log)
        self.clear_button.pack(side=tk.LEFT)

        # Bind Enter key to send command
        self.command_entry.bind("<Return>", lambda event: self.send_command())

    def start_loop(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def update_log(self, message, tag=None):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, f"{message}\n", tag)
        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def clear_log(self):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state=tk.DISABLED)

    def toggle_connection(self):
        if not self.connected:
            # Start connection
            self.connect_button.config(state=tk.DISABLED)
            asyncio.run_coroutine_threadsafe(self.connect_to_device(), self.loop)
        else:
            # Disconnect
            asyncio.run_coroutine_threadsafe(self.disconnect_device(), self.loop)

    def send_command(self):
        command = self.command_entry.get().strip()
        if not command or not self.connected:
            return

        asyncio.run_coroutine_threadsafe(self.write_command(command), self.loop)
        self.command_entry.delete(0, tk.END)

    def display_data(self, sender, data):
        try:
            text = data.decode("utf-8")
        except Exception:
            text = str(data)

        self.root.after(0, lambda: self.update_log(f"Received: {text}"))

    async def connect_to_device(self):
        SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
        CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

        self.root.after(0, lambda: self.update_log("Scanning for BLE-B17 Device..."))

        try:
            devices = await BleakScanner.discover()
            target = None
            for d in devices:
                if d.name and "BLE-B17" in d.name:
                    target = d
                    break

            if not target:
                self.root.after(0, lambda: self.update_log("No BLE-B17 Device Found.", "error"))
                self.root.after(0, lambda: self.connect_button.config(state=tk.NORMAL))
                return

            self.root.after(0, lambda: self.update_log(f"Found {target.name} @Address {target.address}. Connecting..."))

            client = BleakClient(target.address)
            await client.connect()

            if client.is_connected:
                self.client = client
                self.connected = True

                # Start notifications
                await client.start_notify(CHARACTERISTIC_UUID, self.display_data)

                self.root.after(0, lambda: self.update_ui_connected(target))
            else:
                self.root.after(0, lambda: self.update_log("Connection Failed.", "error"))
                self.root.after(0, lambda: self.connect_button.config(state=tk.NORMAL))

        except Exception as e:
            self.root.after(0, lambda: self.update_log(f"Error: {str(e)}", "error"))
            self.root.after(0, lambda: self.connect_button.config(state=tk.NORMAL))

    def update_ui_connected(self, device):
        self.status_label.config(text="Connected", foreground="green")
        self.device_info_label.config(text=f"Connected to: {device.name} ({device.address})")
        self.connect_button.config(text="Disconnect", state=tk.NORMAL)
        self.send_button.config(state=tk.NORMAL)
        self.update_log("Connected successfully!")

    def update_ui_disconnected(self):
        self.status_label.config(text="Disconnected", foreground="red")
        self.device_info_label.config(text="No device connected")
        self.connect_button.config(text="Connect to BLE-B17", state=tk.NORMAL)
        self.send_button.config(state=tk.DISABLED)

    async def disconnect_device(self):
        CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

        if self.client and self.client.is_connected:
            try:
                await self.client.stop_notify(CHARACTERISTIC_UUID)
                await self.client.disconnect()
            except Exception as e:
                self.root.after(0, lambda: self.update_log(f"Error disconnecting: {str(e)}", "error"))

        self.client = None
        self.connected = False
        self.root.after(0, self.update_ui_disconnected)
        self.root.after(0, lambda: self.update_log("Disconnected from device."))

    async def write_command(self, command):
        CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

        if not self.client or not self.client.is_connected:
            self.root.after(0, lambda: self.update_log("Not connected to device.", "error"))
            return

        try:
            self.root.after(0, lambda: self.update_log(f"Sending: {command}"))
            await self.client.write_gatt_char(CHARACTERISTIC_UUID, command.encode("utf-8"))
        except Exception as e:
            self.root.after(0, lambda: self.update_log(f"Error sending command: {str(e)}", "error"))

    def on_closing(self):
        if self.connected:
            asyncio.run_coroutine_threadsafe(self.disconnect_device(), self.loop)

        # Stop the event loop and wait for the thread to finish
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)

        if self.thread:
            self.thread.join(timeout=1.0)

        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = BLEControlApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()