#include "ble.h"
#include "controller.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, STD_BUFFSIZE, false
);

char buffBLE[STD_BUFFSIZE];
bool isAuthenticated = false;
bool pairPrompted = true;

void updateParamBLE(const char* bleBuff) {
  char paramType[STD_BUFFSIZE] = {0};
  char valueStr[STD_BUFFSIZE] = {0};
  float newValue = 0.0f;

  char returnStr[STD_BUFFSIZE] = {0};

  char cmd[STD_BUFFSIZE] = {0};
  strcpy(cmd, bleBuff); // Copy Command to Local Buffer

  int start = 0;
  while ((cmd[start] == ' ') || (cmd[start] == '\t')) start++; // Skip Leading Whitespace

  if (
    strncmp(&cmd[start], "k=", 2) == 0 ||
    strncmp(&cmd[start], "set=", 4) == 0 ||
    strncmp(&cmd[start], "Kp=", 3) == 0 ||
    strncmp(&cmd[start], "Ki=", 3) == 0 ||
    strncmp(&cmd[start], "Kd=", 3) == 0
  ) {
    char* equalsPos = strchr(&cmd[start], '='); // Find '=' Position
    if (!equalsPos) return;

    int paramLength = equalsPos - &cmd[start]; // Length of Parameter Name
    strncpy(paramType, &cmd[start], paramLength); // Copy Parameter Name
    paramType[paramLength] = '\0'; // Null-Termination

    strcpy(valueStr, equalsPos + 1); // Copy Value String
    newValue = atof(valueStr); // Convert Value String to Float

    if (strcmp(paramType, "k") == 0) k = newValue;
    else if (strcmp(paramType, "set") == 0) setpointAngle = newValue;
    else if (strcmp(paramType, "Kp") == 0) Kp = newValue;
    else if (strcmp(paramType, "Ki") == 0) Ki = newValue;
    else if (strcmp(paramType, "Kd") == 0) Kd = newValue;
    else return; // Invalid Command

    sprintf(returnStr, "NewVal: %s=%s", paramType, valueStr);
    customCharacteristic.writeValue(returnStr);
  }
  return; // Exit Function
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

  // Callback Event Handlers
  BLE.setEventHandler(BLEConnected, connectBLE);
  BLE.setEventHandler(BLEDisconnected, disconnectBLE);
  customCharacteristic.setEventHandler(BLEWritten, rxBLE);

  BLE.advertise(); // Advertising the BLE Device

  while (!BLE.connected()) BLE.poll(); // Wait for Connection
  customCharacteristic.writeValue("Pair Device"); // Send Pairing Prompt
}

void connectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, HIGH);
  pairPrompted = false; // Reset Pairing Prompt Flag
}

void disconnectBLE(BLEDevice central) {
  digitalWrite(LED_BUILTIN, LOW);
  isAuthenticated = false; // Reset Authentication Status
}

void authenticateBLE() {
  if (strcmp(buffBLE, CODE) == 0) {
    isAuthenticated = true;
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
    authenticateBLE(); // Authenticate Device
  }
  return; // Exit Function
}