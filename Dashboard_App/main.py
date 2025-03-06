from app import DashboardApp
from convert_videos import VideoConverter

if __name__ == "__main__":
    app = DashboardApp()
    app.run()
    video_converter = VideoConverter()
    video_converter.convert_videos()