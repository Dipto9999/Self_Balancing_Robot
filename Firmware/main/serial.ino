#include "serial.h"

String serialMsg = "";

void setupSerial() {
  Serial.begin(115200);
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
