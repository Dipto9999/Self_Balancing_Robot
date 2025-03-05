from kivy.app import App
from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture

from PIL import Image as PILImage

import datetime as dt

import os
import shutil
import subprocess

from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from libcamera import Transform

class CameraDisplay(Image):
    SAMPLE_RATE = 1.0 / 25.0 # 25 FPS. Increase for Higher Update Rate (CPU permitting)
    def __init__(self, **kwargs):
        super().__init__(size_hint = (1, 1), allow_stretch = True, **kwargs)

        self.filename = ""
        self.camera = Picamera2()

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
            transform = Transform(hflip = True, vflip = True)
        )
        self.camera.configure(self.config)
        self.camera.start()
        # self.start_recording()

    def start_recording(self):
        self.filename = f"Recording_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
        input_file = os.path.join(self.video_dir, f"{self.filename}.h264")

        self.camera.start_recording(H264Encoder(bitrate = 10000000), input_file)

    def stop_recording(self):
        self.camera.stop_recording()

    def take_snapshot(self):
        # Capture Current Frame
        frame = self.camera.capture_array()
        if frame is None or frame.size == 0:
            print("Failed to Capture Snapshot.")
            return

        # Swap Color Channels (BGR -> RGB)
        frame = frame[..., ::-1]

        snapshot_name = f"Snapshot_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
        snapshot_path = os.path.join(self.snapshot_dir, snapshot_name)

        image = PILImage.fromarray(frame) # Convert to PIL Image
        image.save(snapshot_path) # Save Image
        print(f"Snapshot saved to {snapshot_path}")

    def convert_video(self):
        if self.filename == "":
            return

        input_file = os.path.join(self.video_dir, f"{self.filename}.h264")
        temp_file = os.path.join(self.video_dir, "to_convert.h264")
        mp4_file = os.path.join(self.video_dir, f"{self.filename}.mp4")

        shutil.copy2(input_file, temp_file)
        os.remove(input_file)

        # Flip Video Vertically + Horizontally and Copy Audio
        command = [
            "ffmpeg", # Command
            "-i", temp_file, # Input File
            "-vf", "vflip,hflip", # Vertical and Horizontal Flip
            "-c:a", "copy", # Copy Audio
            mp4_file
        ]

        try:
            subprocess.Popen(
                command,
                stdout = subprocess.DEVNULL,
                stderr = subprocess.DEVNULL,
                start_new_session = True
            )
            print("Detached ffmpeg process started.")
        except Exception as e:
            print("Error Starting Detached Conversion:", e)
        finally:
            self.filename = "" # Reset Filename

    def update(self):
        frame = self.camera.capture_array()
        if frame is None or frame.size == 0:
            return

        # Swap Color Channels (BGR -> RGB)
        frame = frame[..., ::-1]

        height, width, _ = frame.shape
        texture = Texture.create(size = (width, height), colorfmt = 'rgb')
        texture.blit_buffer(frame.tobytes(), colorfmt = 'rgb', bufferfmt = 'ubyte')

        self.texture = texture

    def stop(self):
        """Close Camera on Exit."""
        if self.filename == "":
            return

        self.stop_recording()
        self.camera.close()
        self.convert_video()