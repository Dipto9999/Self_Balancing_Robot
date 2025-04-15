import asyncio
import threading as td

from bleak import BleakScanner, BleakClient

import tkinter as tk
from tkinter import scrolledtext, ttk, messagebox

class DesktopApp:
    # Define UUIDs for BLE Service and Characteristic
    SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
    CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"
    PASSWORD = "EVE" # Password & Authentication Code

    def __init__(self, root):
        self.root = root
        self.root.title("WALL-E Command App")

        self.password = None

        self.client = None
        self.connected = False
        self.loop = None
        self.task = None
        self.cmd = None

        self.config_login()
        self.config_app()

        self.asyncio_thread = td.Thread(target = self.start_loop, daemon = True)
        self.asyncio_thread.start()

        self.root.after(500, self.ping)

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self.root.mainloop()

    def lock(self):
        if self.connected:
            asyncio.run_coroutine_threadsafe(self.disconnect(), self.loop)

        # Hide Main Application Frames
        self.conn_frame.pack_forget()
        self.log_frame.pack_forget()
        self.cmd_frame.pack_forget()

        self.login_entry.delete(0, tk.END) # Clear Entry
        self.login_frame.pack(fill = tk.BOTH, expand=True)
        self.login_entry.focus()

    def unlock(self):
        self.conn_frame.pack(fill = tk.X, padx = 10, pady = 10)
        self.connect_button.pack(fill = tk.X, padx = 10, pady = 10)
        self.status_frame.pack(fill=tk.X, padx=10, pady = (0, 10))
        self.status_text.pack(side = tk.LEFT, padx = (0, 5))
        self.status_label.pack(side = tk.LEFT)

        self.log_frame.pack(fill = tk.BOTH, expand = True, padx = 10, pady = 10)
        self.log_text.pack(fill = tk.BOTH, expand = True, padx = 10, pady = 10)

        self.cmd_frame.pack(fill = tk.X, padx = 10, pady = 10)
        self.cmd_entry.pack(fill = tk.X, padx = 10, pady = 10)

        self.button_frame.pack(fill = tk.X, padx = 10, pady = (0, 10))
        self.send_button.pack(side = tk.LEFT, padx = (0, 5))
        self.clear_button.pack(side = tk.LEFT)
        self.lock_button.pack(side = tk.RIGHT, padx = (5, 0))

    def login(self, entered):
        if entered == DesktopApp.PASSWORD:
            self.login_frame.pack_forget()
            self.unlock()
        else:
            messagebox.showerror("Access Denied!", "Incorrect Code!")
            self.login_entry.delete(0, tk.END)  # Clear the entry
            self.login_entry.focus()

    def config_login(self):
        self.login_frame = ttk.Frame(self.root, padding = "20")
        self.login_frame.pack(fill = tk.BOTH, expand = True)

        # Password Label and Entry
        self.login_label = ttk.Label(self.login_frame, text = "Enter Code:", font = ("Arial", 12))
        self.login_entry = ttk.Entry(self.login_frame, show = "*", width = 20)
        self.login_button = ttk.Button(
            self.login_frame,
            text = "Login",
            command = lambda: self.login(self.login_entry.get())
        )

        self.login_label.pack(pady = (10, 5))
        self.login_entry.pack(pady = (0, 10))
        self.login_button.pack(pady = (0, 10))

        self.login_entry.focus()
        self.login_entry.bind("<Return>", lambda event: self.login(self.login_entry.get())) # Bind Enter Key to Login Button

    def config_app(self):
        # Connection Controls Frame
        self.conn_frame = ttk.LabelFrame(self.root, text = "Connection")

        self.connect_button = ttk.Button(
            self.conn_frame,
            text = "Connect to WALL-E",
            command = self.toggle_conn
        )
        self.status_frame = ttk.Frame(self.conn_frame)
        self.status_text = ttk.Label(self.status_frame, text = "Status:")
        self.status_label = ttk.Label(self.status_frame, text = "Disconnected", foreground = "red")

        self.log_frame = ttk.LabelFrame(self.root, text = "Received Data")
        self.log_text = scrolledtext.ScrolledText(
            self.log_frame,
            wrap = tk.WORD,
            height = 10,
            state = tk.DISABLED
        )

        self.cmd_frame = ttk.LabelFrame(self.root, text = "Send CMD")
        self.cmd_entry = ttk.Entry(self.cmd_frame)

        self.button_frame = ttk.Frame(self.cmd_frame)

        self.send_button = ttk.Button(self.button_frame, text = "Send", command = self.send_cmd, state = tk.DISABLED)
        self.clear_button = ttk.Button(self.button_frame, text = "Clear Log", command = self.clear_log)
        self.lock_button = ttk.Button(self.button_frame, text = "Lock App", command = self.lock)

        self.cmd_entry.bind("<Return>", lambda event: self.send_cmd()) # Bind Enter Key to Send Command

    def start_loop(self):
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def ping(self):
        if self.connected and self.client:
            if not self.client.is_connected:
                self.connected = False
                self.display_disconnection()
        self.root.after(500, self.ping)

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
            self.connect_button.config(state = tk.DISABLED)
            asyncio.run_coroutine_threadsafe(self.connect(), self.loop)
        else:
            asyncio.run_coroutine_threadsafe(self.disconnect(), self.loop)

    def send_cmd(self):
        self.cmd = self.cmd_entry.get().strip()
        if (not self.cmd) or (not self.connected) or not (self.loop):
            return

        asyncio.run_coroutine_threadsafe(self.write(self.cmd), self.loop)
        self.cmd_entry.delete(0, tk.END)

    def display_data(self, sender, data):
        try:
            text = data.decode("utf-8")
        except Exception:
            text = str(data)

        if (self.cmd) and (text.strip() == self.cmd):
            self.root.after(750, lambda: self.clear_log())
            self.cmd = None
        else:
            self.root.after(0, lambda: self.update_log(f"Received: {text}"))

    async def connect(self):
        self.root.after(0, lambda: self.update_log("Scanning for WALL-E..."))

        try:
            scanned = await BleakScanner.discover()

            devices = [
                {"name" : device.name, "address" : device.address} for device in scanned if device.name and ("WALL-E" in device.name)
            ] # Filter BLE Devices

            if len(devices) == 0:
                self.root.after(0, lambda: self.update_log("No WALL-E Device Found.", "error"))
                self.root.after(0, lambda: self.connect_button.config(state = tk.NORMAL))
                return

            self.root.after(0, lambda: self.update_log(f"Connecting to {devices[0]['name']} @Address [{devices[0]['address']}]..."))

            client = BleakClient(devices[0]['address'])
            await client.connect()
            if client.is_connected:
                self.client = client
                self.connected = True

                self.ping()

                # Start notifications
                await client.start_notify(DesktopApp.CHARACTERISTIC_UUID, self.display_data)

                # unlock Device
                await self.write(DesktopApp.PASSWORD)
                self.root.after(0, lambda: self.display_connection(devices[0]))
            else:
                self.root.after(0, lambda: self.update_log("Connection Failed.", "error"))
                self.root.after(0, lambda: self.connect_button.config(state=tk.NORMAL))

        except Exception as e:
            self.root.after(0, lambda: self.update_log(f"Error: {str(e)}", "error"))
            self.root.after(0, lambda: self.connect_button.config(state = tk.NORMAL))

    def display_connection(self, device):
        self.status_label.config(text = f"Connected to: {device['name']} ({device['address']})", foreground = "green")
        self.connect_button.config(text = "Disconnect", state = tk.NORMAL)
        self.send_button.config(state = tk.NORMAL)
        self.root.after(750, self.clear_log)

    def display_disconnection(self):
        self.status_label.config(text = "Disconnected", foreground = "red")
        self.connect_button.config(text = "Connect to WALL-E", state=tk.NORMAL)
        self.send_button.config(state = tk.DISABLED)

    async def disconnect(self):
        if self.client and self.client.is_connected:
            try:
                await self.client.stop_notify(DesktopApp.CHARACTERISTIC_UUID)
                await self.client.disconnect()
            except Exception as e:
                self.root.after(0, lambda e=e: self.update_log(f"Error Disconnecting!\n{str(e)}\n", "error"))

        self.client = None
        self.connected = False
        self.root.after(0, self.display_disconnection)
        self.root.after(750, self.clear_log)

    async def write(self, cmd):
        if not self.client or not self.client.is_connected:
            self.root.after(0, lambda: self.update_log("Not Connected to Device.", "error"))
            self.root.after(0, self.display_disconnection)
            return

        try:
            self.root.after(0, lambda: self.update_log(f"Sending: {cmd}"))
            await self.client.write_gatt_char(DesktopApp.CHARACTERISTIC_UUID, cmd.encode("utf-8"))
        except Exception as e:
            self.root.after(0, lambda err=e: self.update_log(f"Error Sending CMD: {str(err)}", "error"))
            # self.toggle_conn()

    def on_close(self):
        if self.connected:
            asyncio.run_coroutine_threadsafe(self.disconnect(), self.loop)
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)
        if self.asyncio_thread:
            self.asyncio_thread.join(timeout = 1.0)

        self.root.destroy()

if __name__ == "__main__":
    app = DesktopApp(tk.Tk())