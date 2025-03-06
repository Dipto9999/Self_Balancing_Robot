import os
import shutil
import subprocess

class VideoConverter:
    def __init__(self):
        self.video_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")
        os.makedirs(self.video_dir, exist_ok = True)

    def convert_videos(self):
        temp_file = os.path.join(self.video_dir, "to_convert.h264")

        for filename in os.listdir(self.video_dir):
            if not filename.endswith(".h264"): continue
            mp4_file = filename.split('.')[0] + ".mp4"

            h264_file = os.path.join(self.video_dir, filename)
            mp4_file = os.path.join(self.video_dir, mp4_file)
            print(f"Converting {filename} to {mp4_file}")

            shutil.copy2(h264_file, temp_file) # Copy File to Convert
            os.remove(h264_file) # Remove Original File

            # Flip Video Vertically + Horizontally and Copy Audio
            command = [
                "ffmpeg", # Command
                "-i", temp_file, # Input File
                "-vf", "vflip,hflip", # Vertical and Horizontal Flip
                "-c:a", "copy", # Copy Audio
                mp4_file
            ]

            try:
                subprocess.run(command, check = True, capture_output = True, text = True)
                print(f"Video Conversion Successful for {mp4_file}")
            except Exception as e:
                print("Error Starting Detached Conversion:", e)

if __name__ == "__main__":
    video_converter = VideoConverter()
    video_converter.convert_videos()