import asyncio
from bleak import BleakScanner, BleakClient

SERVICE_UUID = "00000000-5EC4-4083-81CD-A10B8D5CF6EC"
CHARACTERISTIC_UUID = "00000001-5EC4-4083-81CD-A10B8D5CF6EC"

def notification_handler(sender, data):
    # Decode the received bytes to string and print them
    try:
        text = data.decode("utf-8")
    except Exception:
        text = data
    print(f"Received: {text}")

async def main():
    print("Scanning for BLE-B17 Device...")
    devices = await BleakScanner.discover()
    target = None
    for d in devices:
        if d.name and "BLE-B17" in d.name:
            target = d
            break

    if not target:
        print("No BLE-B17 Device Found.")
        return

    print(f"Found {target.name} @Addr {target.address}. Connecting...")

    async with BleakClient(target.address) as client:
        if not client.is_connected:
            print("Connection Failed.")
            return
        print("Connected.")

        # Start notifications on the characteristic
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        try:
            while True:
                cmd = input("Enter Command: ").strip()
                if cmd.lower() == "exit":
                    break
                try:
                    await client.write_gatt_char(CHARACTERISTIC_UUID, cmd.encode("utf-8"))
                except Exception as e:
                    print(f"Error Sending Command: {e}")
        finally:
            await client.stop_notify(CHARACTERISTIC_UUID)
            print("Stopped Notifications.")

if __name__ == "__main__":
    asyncio.run(main())