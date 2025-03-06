import logging

from app import DashboardApp
from convert_videos import VideoConverter

if __name__ == "__main__":
    logging.getLogger("kivy").setLevel(logging.WARNING)
    logging.getLogger("matplotlib").setLevel(logging.WARNING)
    logging.getLogger("picamera2").setLevel(logging.WARNING)
    logging.getLogger("libcamera").setLevel(logging.WARNING)

    DashboardApp().run()
    VideoConverter().convert_videos()