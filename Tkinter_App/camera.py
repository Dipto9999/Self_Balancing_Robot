import os
import datetime as dt

import tkinter as tk
from PIL import ImageTk
from PIL import Image as PILImage

from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from libcamera import Transform

class CameraDisplay(tk.Frame):
    SAMPLE_RATE = 1.0 / 50.0 # Update Delay ~20 FPS (1000ms/20 ≈ 50ms)
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)

        self.filename = ""
        self.camera = Picamera2() # Init Camera

        self.snapshot_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Snapshots")
        self.video_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")

        os.makedirs(self.snapshot_dir, exist_ok = True)
        os.makedirs(self.video_dir, exist_ok = True)

        # Force RGB888 Output
        self.config = self.camera.create_video_configuration(
            main = {
                "size": (640, 480), # Default Size
                # "size": (1280, 720), # HD
                # "size": (1920, 1080), # Full HD
                "format": "RGB888"
            },
            transform = Transform(hflip = True, vflip = True) # For Tkinter Display
        )
        self.image_label = tk.Label(self)
        self.image_label.pack()

    def start_recording(self):
        self.camera.configure(self.config)
        self.camera.start()

        self.filename = f"Recording_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
        input_file = os.path.join(self.video_dir, f"{self.filename}.h264")

        self.camera.start_recording(H264Encoder(bitrate = 10000000), input_file)

    def stop_recording(self):
        if self.filename == "":
            return

        self.camera.stop_recording()
        self.camera.stop()
        self.filename = ""

    def take_snapshot(self):
        if self.filename == "":
            self.camera.configure(self.config)
            self.camera.start()

        # Capture Current Frame
        frame = self.camera.capture_array()
        if frame is None or frame.size == 0:
            print("Failed to Capture Snapshot.")
            return

        # Swap Color Channels (BGR -> RGB)
        frame = frame[..., ::-1]

        snapshot_name = f"Snapshot_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
        snapshot_path = os.path.join(self.snapshot_dir, snapshot_name)

        # Convert to PIL Image and Flip Vertically
        image = PILImage.fromarray(frame).transpose(PILImage.FLIP_TOP_BOTTOM)
        image.save(snapshot_path) # Save Image
        print(f"Snapshot Saved to {snapshot_path}")

        if self.filename == "":
            self.camera.stop()

    def update(self):
        frame = self.camera.capture_array()
        if (frame is not None) and (frame.size != 0):
            frame = frame[..., ::-1] # BGR -> RGB

            # Convert the PIL Image to an ImageTk.PhotoImage
            self.image_label.current = ImageTk.PhotoImage(
                image = PILImage.fromarray(frame) # Convert Numpy Array to PIL.Image
            )
            self.image_label.configure(image = self.image_label.current)

    def stop(self):
        self.camera.close()

if __name__ == "__main__":
    root = tk.Tk()
    root.title("PiCamera Live Stream")

    camera_frame = CameraDisplay(root)
    camera_frame.pack()

    def on_closing():
        camera_frame.stop()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
