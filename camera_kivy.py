from kivy.app import App
from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture

from picamera2 import Picamera2
from libcamera import Transform

class PiCameraWidget(Image):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.picam2 = Picamera2()

        # If your camera is physically upside down, you might need vflip=True.
        # If the image is mirrored left-to-right, you might want hflip=True.
        # Or set both to False if you don't want any hardware flipping.
        flip_transform = Transform(
            hflip=False,  # set True if you need horizontal flip
            vflip=False   # set True if you need vertical flip
        )

        # Force a true RGB888 output
        config = self.picam2.create_video_configuration(
            main={
                "size": (640, 480),   # or (1280, 720)
                "format": "RGB888"
            },
            transform=flip_transform
        )
        self.picam2.configure(config)
        self.picam2.start()

        # Increase if you want a higher update rate (CPU permitting)
        Clock.schedule_interval(self.update_camera, 1.0 / 20.0)

    def update_camera(self, dt):
        frame = self.picam2.capture_array()
        if frame is None or frame.size == 0:
            return

        # If your face is blue, you likely need to swap BGR -> RGB
        # Uncomment this line to see if it fixes the color:
        frame = frame[..., ::-1]

        height, width, _ = frame.shape
        texture = Texture.create(size=(width, height), colorfmt='rgb')
        texture.blit_buffer(frame.tobytes(), colorfmt='rgb', bufferfmt='ubyte')

        # If the image is still upside-down, you can do:
        texture.flip_vertical()
        # If it's reversed left-to-right, do:
        texture.flip_horizontal()

        self.texture = texture

    def stop(self):
        """Close the camera on exit."""
        self.picam2.close()

class CameraApp(App):
    def build(self):
        return PiCameraWidget()

    def on_stop(self):
        self.root.stop()
        super().on_stop()

if __name__ == "__main__":
    CameraApp().run()
