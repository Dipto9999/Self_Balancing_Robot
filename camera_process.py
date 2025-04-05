import time
import sys
import traceback
import multiprocessing as mp

from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from libcamera import Transform

def camera_run(frame_queue, stop_event, resolution=(640, 480), fps=10):
    """
    Runs in a separate process. Captures frames from Picamera2 at a limited FPS
    and sends them via frame_queue to the main process.
    """
    camera = Picamera2()
    config = camera.create_video_configuration(
        main={
            "size": resolution,
            "format": "RGB888"
        },
        transform=Transform(hflip=False, vflip=False)
    )

    try:
        camera.configure(config)
        camera.start()

        delay = 1.0 / fps

        while not stop_event.is_set():

            frame = camera.capture_array()
            if frame is not None and frame.size != 0:
                frame = frame[..., ::-1]  # BGR -> RGB

                try:
                    frame_queue.put(frame, block=False)
                except:
                    pass
            time.sleep(delay)

    except Exception as e:
        traceback.print_exc()
    finally:
        camera.stop_recording()
        camera.stop()
        camera.close()
        print("[CameraProcess] Exiting camera process.")
