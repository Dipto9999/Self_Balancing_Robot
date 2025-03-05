from kivy.app import App
from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture

import datetime as dt
import os

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
        self.start_recording()

    def start_recording(self):
        self.filename = f"Recording_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"

        video_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")
        input_file = os.path.join(video_dir, f"{self.filename}.h264")

        self.camera.start_recording(H264Encoder(bitrate = 10000000), input_file)

    def stop_recording(self):
        self.camera.stop_recording()

    def convert_video(self):
        if self.filename == "":
            return

        video_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")

        input_file = os.path.join(video_dir, f"{self.filename}.h264")
        temp_file = os.path.join(video_dir, "temp.h264")
        mp4_file = os.path.join(video_dir, f"{self.filename}.mp4")

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

class TestApp(App):
    def build(self):
        self.cam_feed = CameraDisplay()

        Clock.schedule_interval(self.update_camera, CameraDisplay.SAMPLE_RATE)
        return self.cam_feed

    def update_camera(self, dt):
        self.cam_feed.update()

    def on_stop(self):
        self.root.stop()
        super().on_stop()

if __name__ == "__main__":
    TestApp().run()
