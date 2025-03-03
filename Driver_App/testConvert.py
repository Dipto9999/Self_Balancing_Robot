import os
import subprocess

def convert_video(filename):
    input_file = f"{filename}.h264"
    logbook_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Logbook")
    input_file = os.path.join(logbook_dir, input_file)

    # Output File Paths
    mp4_file = os.path.join(logbook_dir, filename + ".mp4")
    temp_file = os.path.join(logbook_dir, "converted.mp4")

    # Flip Video Vertically + Horizontally and Copy Audio
    command = [
        "ffmpeg", # Command
        "-i", input_file, # Input File
        "-vf", "vflip,hflip", # Vertical and Horizontal Flip
        "-c:a", "copy", # Copy Audio
        temp_file
    ]

    print(input_file)

    try:
        result = subprocess.run(command, check = True, capture_output = True, text = True) # Run Command
        print("ffmpeg Output:", result.stdout)
        os.replace(temp_file, mp4_file)  # Replace the Temporary File with the MP4 File
        os.remove(input_file) # Remove the original H264 File
        print(f"Conversion Successful. Video Saved as {mp4_file}")
    except subprocess.CalledProcessError as e:
        print("ffmpeg Conversion Failed:", e)
    except OSError as e:
        print("File Operation Failed:", e)

convert_video("Camera_Data_20250303_122913")
