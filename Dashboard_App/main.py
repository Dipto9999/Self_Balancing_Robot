from app import DashboardApp
from convert_videos import VideoConverter

import multiprocessing as mp

if __name__ == "__main__":
    app = DashboardApp()
    video_converter = VideoConverter()

    app_process = mp.Process(target = app.run, daemon = True)
    conversion_process = mp.Process(target = video_converter.convert_videos, daemon = False)

    app_process.start()
    app_process.join()

    conversion_process.start()
    conversion_process.join()