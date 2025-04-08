import logging

from app import DashboardApp
from convert_videos import VideoConverter
from s3 import S3Manager

if __name__ == "__main__":
    logging.getLogger("matplotlib").setLevel(logging.WARNING)
    logging.getLogger("picamera2").setLevel(logging.WARNING)
    logging.getLogger("libcamera").setLevel(logging.WARNING)

    DashboardApp().mainloop()

    vidConverter = VideoConverter()
    vidConverter.convert_videos()

    s3_manager = S3Manager()
    s3_manager.upload_files(s3_manager.figures_dir)
    s3_manager.upload_files(s3_manager.logbook_dir)
    s3_manager.upload_files(s3_manager.snapshot_dir)
    s3_manager.upload_files(s3_manager.video_dir)
    # s3_manager.download_files()