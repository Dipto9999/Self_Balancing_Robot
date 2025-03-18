import tkinter as tk
from PIL import Image, ImageTk

from picamera2 import Picamera2
from libcamera import Transform

class CameraFrame(tk.Frame):
    DELAY_MS = 50 # Update Delay ~20 FPS (1000ms/20 ≈ 50ms)
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)

        self.picam2 = Picamera2() # Init Camera
        flip_transform = Transform(hflip = False, vflip = False)

        # Force a True RGB888 Output
        config = self.picam2.create_video_configuration(
            main = {
                "size": (640, 480),
                "format": "RGB888"
            },
            transform = flip_transform
        )
        self.picam2.configure(config)
        self.picam2.start()

        self.image_label = tk.Label(self)
        self.image_label.pack()

        self.update_camera()

    def update_camera(self):
        frame = self.picam2.capture_array()
        if (frame is not None) and (frame.size != 0):
            frame = frame[..., ::-1] # BGR -> RGB

            # Convert the PIL Image to an ImageTk.PhotoImage
            self.image_label.current = ImageTk.PhotoImage(
                image = Image.fromarray(frame) # Convert Numpy Array to PIL.Image
            )
            self.image_label.configure(image = self.image_label.current)
        self.after(CameraFrame.DELAY_MS, self.update_camera)

    def stop(self):
        self.picam2.close()

if __name__ == "__main__":
    root = tk.Tk()
    root.title("PiCamera Live Stream")

    camera_frame = CameraFrame(root)
    camera_frame.pack()

    def on_closing():
        camera_frame.stop()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
