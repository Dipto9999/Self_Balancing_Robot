#include "ble.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, BUFFER_SIZE, false
);

BLEDevice central;
bool connectSuccess = false;

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT); // Init Built-in LED to Indicate Connection Status

  delay(2000); // Delay to Allow Serial Monitor to Connect
  if (!BLE.begin()) {
    Serial.println("Starting BLE Failed!");
    while (1);
  }

  // Set Local Name and Device Name
  BLE.setLocalName("BLE-B17");
  BLE.setDeviceName("BLE-B17");

  customService.addCharacteristic(customCharacteristic);  // Add the Characteristic to the Service
  BLE.addService(customService); // Add the Service to the BLE Device
  customCharacteristic.writeValue("BLE Rdy"); // Set Initial Value for the Characteristic

  BLE.advertise(); // Advertising the BLE Device
  Serial.println("Bluetooth® Device Active | Waiting for Connections...");
}

char buffBLE[BUFFER_SIZE];

bool handleBLE() {
  central = BLE.central();

  while (central) { // While Central Device is Detected
    connectSuccess = true;
    if (central.connected()) { // If Device is Actively Connected
      if (customCharacteristic.written()) {
        int length = customCharacteristic.valueLength();
        const unsigned char* receivedData = customCharacteristic.value();

        memcpy(buffBLE, receivedData, length);
        buffBLE[length] = '\0'; // Null-Terminated

        // Serial.print("Received Data: ");
        // Serial.println(buffBLE);

        customCharacteristic.writeValue("BLE RX"); // Send Acknowledgement
        return true; // Data Received
      }
    }
  }
  digitalWrite(LED_BUILTIN, HIGH); // Turn On LED when Disconnected
  return false; // Disconnected
}