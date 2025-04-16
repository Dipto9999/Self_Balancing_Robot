#include "serial.h"

String serialMsg = "";

void setupSerial() {
  Serial1.begin(115200);
  Serial1.println("Serial Initialized!");
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