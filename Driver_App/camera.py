from kivy.app import App
from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture

from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from libcamera import Transform

import subprocess
import os

import datetime as dt

class CameraDisplay(Image):
    SAMPLE_RATE = 1.0 / 25.0 # 25 FPS. Increase for Higher Update Rate (CPU permitting)
    def __init__(self, **kwargs):
        super().__init__(size_hint = (1, 1), allow_stretch = True, **kwargs)

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

        self.recording_path = f"Logbook/Camera_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
        self.start_recording(f"{self.recording_path}.h264")

    def start_recording(self, filename):
        self.camera.start_recording(H264Encoder(bitrate = 10000000), filename)

    def stop_recording(self):
        self.camera.stop_recording()

    def convert_video(self, input_file):
        base_dir = os.path.dirname(os.path.abspath(__file__))
        input_file = os.path.join(base_dir, input_file)

        # Output File Paths
        mp4_file = os.path.join(base_dir, self.recording_path + ".mp4")
        temp_file = os.path.join(base_dir, "converted.mp4")

        # Flip Video Vertically + Horizontally and Copy Audio
        command = [
            "ffmpeg", # Command
            "-i", input_file, # Input File
            "-vf", "vflip,hflip", # Vertical and Horizontal Flip
            "-c:a", "copy", # Copy Audio
            temp_file
        ]

        try:
            result = subprocess.run(command, check = True, capture_output = True, text = True) # Run Command
            print("ffmpeg Output:", result.stdout)
            os.replace(temp_file, mp4_file)  # Replace the Temporary File with the MP4 File
            os.remove(input_file) # Remove the original H264 File
            print(f"Conversion Successful. Video Saved as {mp4_file}")
        except subprocess.CalledProcessError as e:
            print("ffmpeg Conversion Failed:", e)
        except OSError as e:
            print("File Operation Failed:", e)

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
        self.stop_recording()
        self.camera.close()
        Clock.schedule_once(lambda dt: self.convert_video(f"{self.recording_path}.h264"), 0.5) # Convert Video

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
