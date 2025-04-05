#include "serial.h"

String serialMsg = "";

void setupSerial() {
  Serial1.begin(115200);
}

void handleData(char expectedByte, String txData) {
  // Send Data When Rx Byte Received
  if (Serial1.available() > 0) {
    // Read Incoming Byte
    int rxByte = Serial1.read();
    // Receive Byte rxByte to Send Angle Data
    if (rxByte == expectedByte) {
      Serial1.println(txData);
    }
  }
}

void updatePID() {
  int parseIndex;
  String paramType, valueStr;
  if (Serial1.available() > 0) {
    // Check for PID Parameter Update Command
    String command = Serial1.readStringUntil('\n');
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
        Serial1.println("Invalid Value!");
        return;
      }

      if (paramType == "k") k = newValue;
      else if (paramType == "set") setpointAngle = newValue;
      else if (paramType == "Kp") Kp = newValue;
      else if (paramType == "Ki") Ki = newValue;
      else if (paramType == "Kd") Kd = newValue;

      Serial1.print("\r\nk: ");
      Serial1.print(k, 3);
      Serial1.print(" Setpoint: ");
      Serial1.println(setpointAngle, 3);

      Serial1.print("Kp: ");
      Serial1.print(Kp, 4);
      Serial1.print(" Ki: ");
      Serial1.print(Ki, 4);
      Serial1.print(" Kd: ");
      Serial1.print(Kd, 4);
    }
  }
}
