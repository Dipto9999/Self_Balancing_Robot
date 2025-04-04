#include "ble.h"
#include "controller.h"

// Define a Custom BLE Service and Characteristic.
BLEService customService("00000000-5EC4-4083-81CD-A10B8D5CF6EC");
BLECharacteristic customCharacteristic(
    "00000001-5EC4-4083-81CD-A10B8D5CF6EC",
    BLERead | BLEWrite | BLENotify, BUFFER_SIZE, false
);

char buffBLE[BUFFER_SIZE];

// void updateParamBLE(const char* bleBuff) {
  // int parseIndex;
  // String paramType, valueStr;

  // Check for PID Parameter Update Command

  // String cmd = String(bleBuff);
  // cmd.trim(); // Remove Leading/Trailing Whitespace

  // if (
    // cmd.startsWith("k=") ||
    // cmd.startsWith("set=") ||
    // cmd.startsWith("Kp=") ||
    // cmd.startsWith("Ki=") ||
    // cmd.startsWith("Kd=")
  // ) {
      // parseIndex = cmd.indexOf('=');
      // paramType = cmd.substring(0, parseIndex);
      // valueStr = cmd.substring(parseIndex + 1);

      // float newValue = valueStr.toFloat();

      // if (paramType == "k") k = newValue;
      // else if (paramType == "set") setpointAngle = newValue;
      // else if (paramType == "Kp") Kp = newValue;
      // else if (paramType == "Ki") Ki = newValue;
      // else if (paramType == "Kd") Kd = newValue;
      // else return; // Invalid Command

      // customCharacteristic.writeValue(paramType.c_str());
      // customCharacteristic.writeValue(valueStr.c_str());
  // } else {
    // return; // Invalid Command
  // }
// }

void updateParamBLE(const char* bleBuff) {
  char paramType[BUFFER_SIZE] = {0};
  char valueStr[BUFFER_SIZE] = {0};
  float newValue = 0.0f;

  char cmd[BUFFER_SIZE] = {0};
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

    customCharacteristic.writeValue(paramType);
    customCharacteristic.writeValue(valueStr);
  } else {
    return; // Invalid Command
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
  while (!BLE.connected()) BLE.poll(); // Wait for Connection
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

  // updateParamBLE(buffBLE); // Update PID Parameters based on BLE Input
  changeDirection(buffBLE); // Change Direction based on BLE Input
}