#include "serial.h"

String serialMsg = "";

void setupSerial() {
  Serial.begin(115200);
  // Serial.println("Serial Initialized!");
}

void handleData(char expectedByte, String txData) {
  // Send Data When Rx Byte Received
  if (Serial.available() > 0) {
    // Read Incoming Byte
    int rxByte = Serial.read();
    // Receive Byte rxByte to Send Angle Data
    if (rxByte == expectedByte) {
      Serial.println(txData);
    }
  }
}

void updatePID() {
  int parseIndex;
  String paramType, valueStr;
  if (Serial.available() > 0) {
    // Check for PID Parameter Update Command
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove Leading/Trailing Whitespace

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

      if (valueStr != "0" && newValue == 0) {
        Serial.println("Invalid Value!");
        return;
      }

      if (paramType == "k") k = newValue;
      else if (paramType == "set") SETPOINT_0 = newValue;
      else if (paramType == "Kp") Kp = newValue;
      else if (paramType == "Ki") Ki = newValue;
      else if (paramType == "Kd") Kd = newValue;

      Serial.print("\r\nk: ");
      Serial.print(k, 3);
      Serial.print(" Setpoint: ");
      Serial.println(setpointAngle, 3);

      Serial.print("Kp: ");
      Serial.print(Kp, 4);
      Serial.print(" Ki: ");
      Serial.print(Ki, 4);
      Serial.print(" Kd: ");
      Serial.print(Kd, 4);
    }
  }
}
