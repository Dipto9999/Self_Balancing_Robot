import os
import secrets

from dotenv import load_dotenv

class KeyGenerator:
    ENV_PATH = ".env"
    KEY_LENGTH = 32 # Number of Bytes
    def __init__(self):
        if not self.keyExists():
            self.generateKey()
        self.cleanEnvFile()
        load_dotenv(self.ENV_PATH) # Load Environment Variables from .env file

    def keyExists(self):
        "Check if Key Exists"
        if os.path.exists(KeyGenerator.ENV_PATH):
            with open(KeyGenerator.ENV_PATH, "r") as file:
                for line in file:
                    parsed = line.strip()
                    if parsed.startswith("FLASK_SECRET_KEY="):
                        key = parsed.split("=", 1)[1]
                        return self.isValid(key)
        return False

    def isValid(self, key):
        "Check if Key is Valid"

        secure_key: bool = (len(key) == KeyGenerator.KEY_LENGTH * 2) # 32 Bytes
        hex_key: bool = all(c in "0123456789abcdef" for c in key.lower())

        return secure_key and hex_key

    def generateKey(self):
        new_key = secrets.token_hex(KeyGenerator.KEY_LENGTH)
        with open(KeyGenerator.ENV_PATH, "a") as f:
            f.write(f"\nFLASK_SECRET_KEY={new_key}\n")
        os.environ["FLASK_SECRET_KEY"] = new_key

    def cleanEnvFile(self):
        "Remove Garbage/Empty Lines from .env file"
        with open(KeyGenerator.ENV_PATH, "r") as file:
            lines = file.readlines()
        with open(KeyGenerator.ENV_PATH, "w") as file:
            for line in lines:
                if (line.strip()) and (line.find("=") != -1) and (not line.startswith("#")) :
                    file.write(line)