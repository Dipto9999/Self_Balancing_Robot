#include "ble.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, STD_BUFFSIZE, false
);

char buffBLE[STD_BUFFSIZE];

bool isAuthenticated = false;
bool isPromptedBLE = true;

unsigned long lastBLETime = 0;
const unsigned long BLE_INTERVAL = 100;

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT); // Init Built-in LED to Indicate Connection Status

  if (!BLE.begin()) {
    Serial.println("Starting BLE Failed!");
    while (1);
  }

  // Set Local Name and Device Name
  BLE.setLocalName("WALL-E");
  BLE.setDeviceName("WALL-E");

  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(customCharacteristic);  // Add the Characteristic to the Service
  BLE.addService(customService); // Add the Service to the BLE Device

  // Callback Event Handlers
  BLE.setEventHandler(BLEConnected, connectBLE);
  BLE.setEventHandler(BLEDisconnected, disconnectBLE);
  customCharacteristic.setEventHandler(BLEWritten, rxBLE);

  BLE.advertise(); // Advertising the BLE Device

  while (!BLE.connected()) BLE.poll(); // Wait for Connection
  customCharacteristic.writeValue("Enter Code:"); // Send BLE Prompt
}

void connectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, HIGH);
  isAuthenticated = false; // Reset Authentication Status
  isPromptedBLE = false; // Reset BLE Prompt Flag
}

void disconnectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, LOW);
}

void authBLE() {
  if (strcmp(buffBLE, CODE) == 0) {
    customCharacteristic.writeValue("Auth Success!");
    isAuthenticated = true; // Set Authentication Status
  } else {
    customCharacteristic.writeValue("Auth Fail!");
    BLE.disconnect(); // Disconnect if Authentication Fails
  }
}

void rxBLE(BLEDevice central, BLECharacteristic characteristic) {
  int length = characteristic.valueLength();
  const unsigned char* receivedData = characteristic.value();

  memcpy(buffBLE, receivedData, length);
  buffBLE[length] = '\0'; // Null-Terminated

  if (isAuthenticated) {
    // updateParamBLE(buffBLE); // Update PID Parameters Based On BLE Input
    changeDirection(buffBLE); // Change Direction Based On BLE Input
  } else {
    authBLE(); // Authenticate Device
  }
  return;
}

void checkEventBLE() {
  unsigned long currentMillis = millis();
  // Wait for BLE Connection to Override Motors
  if (currentMillis - lastBLETime >= BLE_INTERVAL) {
    lastBLETime = currentMillis;
    BLE.poll(); // Poll the BLE Device
  }
}