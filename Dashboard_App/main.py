import logging

from app import DashboardApp
from convert_videos import VideoConverter
from bucket_upload import S3Uploader

if __name__ == "__main__":
    logging.getLogger("matplotlib").setLevel(logging.WARNING)
    logging.getLogger("picamera2").setLevel(logging.WARNING)
    logging.getLogger("libcamera").setLevel(logging.WARNING)

    DashboardApp().mainloop()

    vidConverter = VideoConverter()
    vidConverter.convert_videos()

    file_uploader = S3Uploader()
    file_uploader.upload_files(file_uploader.figures_dir)
    file_uploader.upload_files(file_uploader.logbook_dir)
    file_uploader.upload_files(file_uploader.video_dir)
    file_uploader.upload_files(file_uploader.snapshot_dir)