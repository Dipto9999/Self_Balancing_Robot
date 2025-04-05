import sys
import time
import multiprocessing as mp

import tkinter as tk
from tkinter import ttk

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

from PIL import Image, ImageTk

from camera_process import camera_run
from stripchart_process import serial_run

class CameraDisplay(tk.Label):
    """
    Thin wrapper that displays images coming from a multiprocessing.Queue.
    """
    def __init__(self, master, frame_queue, *args, **kwargs):
        matplotlib.use("TkAgg")
        super().__init__(master, *args, **kwargs)
        self.frame_queue = frame_queue
        self.current_image = None

    def update_display(self):
        try:
            frame = None
            while not self.frame_queue.empty():
                frame = self.frame_queue.get_nowait()
            if frame is not None:
                pil_img = Image.fromarray(frame)  # frame is RGB
                self.current_image = ImageTk.PhotoImage(pil_img)
                self.config(image=self.current_image)
        except:
            pass

class DashboardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot Dashboard (Multiprocessing)")

        # We'll use two queues:
        # 1) frame_queue for camera frames
        # 2) data_queue for lines from the serial port
        self.frame_queue = mp.Queue(maxsize=5)  # keep a small maxsize to avoid memory blow
        self.data_queue = mp.Queue(maxsize=50)

        # We'll also have stop events to signal child processes to exit
        self.camera_stop = mp.Event()
        self.stripchart_stop = mp.Event()

        # Spawn child processes
        # NOTE: reduce resolution/fps for better performance on Pi Zero 2
        self.camera_proc = mp.Process(
            target=camera_run,
            args=(self.frame_queue, self.camera_stop, (320, 240), 5),
            daemon=True
        )
        self.stripchart_proc = mp.Process(
            target=serial_run,
            args=(self.data_queue, self.stripchart_stop, '/dev/ttyS0', 115200),
            daemon=True
        )

        self.camera_proc.start()
        self.stripchart_proc.start()

        # Setup UI frames
        self.dashboard_frame = tk.Frame(self, bg='#000000')
        self.camera_frame = tk.Frame(self.dashboard_frame, bg='#141654')
        self.stripchart_frame = tk.Frame(self.dashboard_frame, bg='#48484d')

        # Create a camera display widget that only reads from self.frame_queue
        self.cam_feed = CameraDisplay(self.camera_frame, self.frame_queue, bg='#141654')
        self.cam_feed.pack(padx=10, pady=10)

        self.camera_frame.grid(row=0, column=0, padx=10, pady=10, sticky=tk.NSEW)

        # Create a simple stripchart with matplotlib
        # We'll store incoming data in lists, then periodically update
        self.fig, self.ax = plt.subplots(figsize=(5,4))
        self.ax.set_ylim([-90, 90])
        self.ax.set_xlim([0, 50])
        self.ax.set_xlabel("Sample #")
        self.ax.set_ylabel("Angle (Deg)")

        self.acc_line, = self.ax.plot([], [], label="Accelerometer")
        self.gyr_line, = self.ax.plot([], [], label="Gyroscope")
        self.com_line, = self.ax.plot([], [], label="Complementary")

        self.ax.legend()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.stripchart_frame)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack()

        self.stripchart_frame.grid(row=0, column=1, padx=10, pady=10, sticky=tk.NSEW)
        self.dashboard_frame.pack(fill=tk.BOTH, expand=True)

        # We'll keep track of data in the main app
        self.sample_count = 0
        self.acc_data = []
        self.gyr_data = []
        self.com_data = []
        self.sample_index = []

        # Schedule periodic updates
        self.update_camera()
        self.update_stripchart()

        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def update_camera(self):
        """
        Called periodically to fetch frames from frame_queue and update the label.
        """
        self.cam_feed.update_display()
        # Re-run this after 100ms or so
        self.after(100, self.update_camera)

    def update_stripchart(self):
        """
        Called periodically to fetch any lines from data_queue and update the plot.
        """
        try:
            while not self.data_queue.empty():
                line = self.data_queue.get_nowait()
                # parse the line, e.g. "1.23 2.34 3.45"
                parts = line.split()
                if len(parts) == 3:
                    try:
                        acc = float(parts[0])
                        gyr = float(parts[1])
                        com = float(parts[2])
                        self.sample_count += 1
                        self.acc_data.append(acc)
                        self.gyr_data.append(gyr)
                        self.com_data.append(com)
                        self.sample_index.append(self.sample_count)
                    except:
                        pass
        except:
            pass

        # Now update the lines on the matplotlib figure
        self.acc_line.set_data(self.sample_index, self.acc_data)
        self.gyr_line.set_data(self.sample_index, self.gyr_data)
        self.com_line.set_data(self.sample_index, self.com_data)

        # Adjust X-limits if we exceed 50 samples, etc.
        if self.sample_count > 50:
            self.ax.set_xlim([self.sample_count - 50, self.sample_count])

        self.canvas.draw()
        # Re-run this after 500ms
        self.after(500, self.update_stripchart)

    def on_close(self):
        """
        Stop child processes, then close.
        """
        self.camera_stop.set()
        self.stripchart_stop.set()
        # Give them a moment to exit
        time.sleep(0.5)
        # Terminate if still alive
        if self.camera_proc.is_alive():
            self.camera_proc.terminate()
        if self.stripchart_proc.is_alive():
            self.stripchart_proc.terminate()

        self.destroy()
        sys.exit()

if __name__ == "__main__":
    mp.set_start_method("spawn")  # safer on some OSes / Pi
    app = DashboardApp()
    app.mainloop()
