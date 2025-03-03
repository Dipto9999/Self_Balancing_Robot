from kivy.app import App
from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture

from picamera2 import Picamera2
from libcamera import Transform

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
        self.camera.start_recording(f"Camera_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}.h264")

    def start_recording(self, filename):
        self.camera.start_recording(filename)

    def stop_recording(self):
        self.camera.stop_recording()

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
        self.camera.stop_recording()
        self.camera.close()

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
