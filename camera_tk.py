import tkinter as tk
from PIL import Image, ImageTk
from picamera2 import Picamera2
from libcamera import Transform
from PIL import Image  # Needed for converting arrays to images

class CameraFrame(tk.Frame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        # Initialize the camera
        self.picam2 = Picamera2()

        # Adjust flip settings as needed:
        flip_transform = Transform(
            hflip=False,  # set True to mirror horizontally
            vflip=False   # set True to flip vertically
        )

        # Set the configuration to output RGB888 at 640x480 (or adjust as needed)
        config = self.picam2.create_video_configuration(
            main={
                "size": (640, 480),
                "format": "RGB888"
            },
            transform=flip_transform
        )
        self.picam2.configure(config)
        self.picam2.start()

        # Create a label within this frame to display the video
        self.image_label = tk.Label(self)
        self.image_label.pack()

        # Set update delay for ~20 FPS (1000ms/20 ≈ 50ms)
        self.delay = 50
        self.update_camera()

    def update_camera(self):
        # Capture a frame from the camera
        frame = self.picam2.capture_array()
        if frame is None or frame.size == 0:
            self.after(self.delay, self.update_camera)
            return

        # If necessary, swap BGR -> RGB (adjust based on your camera's output)
        frame = frame[..., ::-1]

        # Convert the frame (a NumPy array) into a PIL Image
        img = Image.fromarray(frame)

        # For additional flipping if needed:
        # img = img.transpose(Image.FLIP_TOP_BOTTOM)    # vertical flip
        # img = img.transpose(Image.FLIP_LEFT_RIGHT)      # horizontal flip

        # Convert the PIL Image to an ImageTk.PhotoImage
        imgtk = ImageTk.PhotoImage(image=img)

        # Update the label with the new image; keep a reference to avoid garbage collection
        self.image_label.imgtk = imgtk
        self.image_label.configure(image=imgtk)

        # Schedule the next frame update
        self.after(self.delay, self.update_camera)

    def stop(self):
        """Close the camera when stopping the application."""
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
