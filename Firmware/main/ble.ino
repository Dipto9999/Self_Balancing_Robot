#include "ble.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, BUFFER_SIZE, false
);

char buffBLE[BUFFER_SIZE];

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT); // Init Built-in LED to Indicate Connection Status

  if (!BLE.begin()) {
    Serial.println("Starting BLE Failed!");
    while (1);
  }

  // ToDo: Investigate BLE Configuration
  // BLE.setAdvertisingInterval(800); // Advertising Interval (500ms)
  // BLE.setConnectionInterval(0x0028, 0x0064); // Connection Interval (50ms - 100ms)

  // Set Local Name and Device Name
  BLE.setLocalName("BLE-B17");
  BLE.setDeviceName("BLE-B17");

  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(customCharacteristic);  // Add the Characteristic to the Service
  BLE.addService(customService); // Add the Service to the BLE Device

  customCharacteristic.writeValue("BLE Rdy"); // Set Initial Value for the Characteristic

  // ----- Callback Event Handlers ----- //
  BLE.setEventHandler(BLEConnected, connectBLE);
  BLE.setEventHandler(BLEDisconnected, disconnectBLE);
  customCharacteristic.setEventHandler(BLEWritten, rxBLE);

  BLE.advertise(); // Advertising the BLE Device
}

void connectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void disconnectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, LOW);
}

void rxBLE(BLEDevice central, BLECharacteristic characteristic) {
  int length = characteristic.valueLength();
  const unsigned char* receivedData = characteristic.value();

  memcpy(buffBLE, receivedData, length);
  buffBLE[length] = '\0'; // Null-Terminated

  changeDirection(buffBLE); // Change Direction based on BLE Input
}