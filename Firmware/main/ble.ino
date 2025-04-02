#include "ble.h"
#include "controller.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, BUFFER_SIZE, false
);

char buffBLE[BUFFER_SIZE];

void updateParamBLE(const char* bleBuff) {
  int parseIndex;
  String paramType, valueStr;

  // Check for PID Parameter Update Command

  String command = String(bleBuff);
  command.trim(); // Remove Leading/Trailing Whitespace

  // Check if command starts with a valid prefix
  if (
    command.startsWith("k=") ||
    command.startsWith("set=") ||
    command.startsWith("Kp=") ||
    command.startsWith("Ki=") ||
    command.startsWith("Kd=")
  ) {
      parseIndex = command.indexOf('=');
      paramType = command.substring(0, parseIndex);
      valueStr = command.substring(parseIndex + 1);

      float newValue = valueStr.toFloat();

      if (valueStr != "0") return;

      if (paramType == "k") k = newValue;
      else if (paramType == "set") setpointAngle = newValue;
      else if (paramType == "Kp") Kp = newValue;
      else if (paramType == "Ki") Ki = newValue;
      else if (paramType == "Kd") Kd = newValue;
      else return; // Invalid Command

      customCharacteristic.writeValue(paramType.c_str());
      customCharacteristic.writeValue(valueStr.c_str());
  } else {
    changeDirection(buffBLE); // Change Direction based on BLE Input
  }
}

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT); // Init Built-in LED to Indicate Connection Status

  if (!BLE.begin()) {
    Serial.println("Starting BLE Failed!");
    while (1);
  }

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
  while (!BLE.connected()); // Wait for Connection
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

  updateParamBLE(buffBLE); // Update PID Parameters based on BLE Input
}