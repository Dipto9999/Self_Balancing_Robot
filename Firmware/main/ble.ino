#include "ble.h"
#include "serial.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, BUFFER_SIZE, false
);

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT); // Init Built-in LED to Indicate Connection Status

  delay(2000); // Delay to Allow Serial Monitor to Connect
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Set Local Name and Device Name
  BLE.setLocalName("BLE-B17");
  BLE.setDeviceName("BLE-B17");

  customService.addCharacteristic(customCharacteristic);  // Add the Characteristic to the Service
  BLE.addService(customService); // Add the Service to the BLE Device
  customCharacteristic.writeValue("Waiting for Data"); // Set Initial Characteristic Value

  BLE.advertise(); // Advertising the BLE Device
  Serial.println("Bluetooth® Device Active | Waiting for Connections...");
}

void handleBLE() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to Central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH); // Turn on LED to Indicate Connection

    while (central.connected()) {
      if (customCharacteristic.written()) {
        int length = customCharacteristic.valueLength();
        const unsigned char* receivedData = customCharacteristic.value();

        char receivedString[length + 1];
        memcpy(receivedString, receivedData, length);
        receivedString[length] = '\0'; // Null-Terminated

        Serial.print("Received Data: ");
        Serial.println(receivedString);

        customCharacteristic.writeValue("Data Received");
      }
    }

    digitalWrite(LED_BUILTIN, LOW); // Turn off LED when Disconnected
    Serial.println("Disconnected from Central.");
  }
}
