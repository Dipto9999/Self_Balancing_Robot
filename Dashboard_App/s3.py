import os
import boto3

class S3Manager:
    def __init__(self, bucket_name = "self-balancing-robot", region_name = 'us-west-1'):
        self.s3_client = boto3.client('s3', region_name = region_name)
        self.bucket_name = bucket_name

        self.figures_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Figures")
        self.logbook_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Logbook")

        self.snapshot_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Snapshots")
        self.video_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Videos")

        self.S3_dir: str = os.path.join(os.path.dirname(os.path.abspath(__file__)), "S3")

    def upload_file(self, file_path):
        try:
            file_name = os.path.basename(file_path)
            self.s3_client.upload_file(file_path, self.bucket_name, file_name)
            print(f"Uploaded {file_name} to {self.bucket_name}")
        except Exception as e:
            print(f"Upload Failed For {file_path}: {e}")

    def upload_files(self, directory):
        for root, _, files in os.walk(directory):
            for file in files:
                file_path = os.path.join(root, file)
                self.upload_file(file_path)

    def download_files(self):
        response = self.s3_client.list_objects_v2(Bucket = self.bucket_name)
        if 'Contents' in response:
            for obj in response['Contents']:
                file_name = obj['Key']
                self.s3_client.download_file(self.bucket_name, file_name, os.path.join(self.S3_dir, file_name))
                print(f"Downloaded {file_name} from {self.bucket_name}")

if __name__ == "__main__":
    s3_manager = S3Manager()
    # s3_manager.upload_files(s3_manager.figures_dir)
    # s3_manager.upload_files(s3_manager.logbook_dir)
    # s3_manager.upload_files(s3_manager.snapshot_dir)
    # s3_manager.upload_files(s3_manager.video_dir)
    s3_manager.download_files()
