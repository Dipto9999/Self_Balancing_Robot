import asyncio
from bleak import BleakClient, BleakScanner

# Service and characteristic UUIDs as defined in your BLE firmware.
SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

# The advertised name of your BLE device.
TARGET_NAME = "BLE-B17"

async def run():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    target_device = None
    for device in devices:
        print(f"Found device: {device.name} - {device.address}")
        if device.name == TARGET_NAME:
            target_device = device
            break

    if not target_device:
        print(f"Device with name {TARGET_NAME} not found!")
        return

    print(f"Connecting to {target_device.name} ({target_device.address})...")
    async with BleakClient(target_device.address) as client:
        if client.is_connected:
            print("Connected!")
            # List of commands to send.
            commands = ["A", "B", "C", "D"]
            for cmd in commands:
                print(f"Sending command: {cmd}")
                # Write the command (as a UTF-8 encoded byte string) to the characteristic.
                await client.write_gatt_char(CHARACTERISTIC_UUID, cmd.encode("utf-8"))
                # Wait a moment between commands.
                await asyncio.sleep(1)
        else:
            print("Failed to connect.")

if __name__ == "__main__":
    asyncio.run(run())
