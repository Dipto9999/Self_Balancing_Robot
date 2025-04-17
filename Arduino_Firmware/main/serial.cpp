#include "serial.h"

String serialMsg = "";

void setupSerial() {
  Serial.begin(115200); // Initialize Serial Monitor
  Serial1.begin(115200); // Initialize UART1 for Raspberry Pi
  Serial.println("Serial Initialized!");
}

void handleData(char expectedByte, String txData) {
  // Send Data When Rx Byte Received
  if (Serial1.available() > 0) {
    // Read Incoming Byte
    int rxByte = Serial1.read();
    // Receive Byte rxByte to Send Angle Data
    if (rxByte == expectedByte) {
      // Serial.print("RX Byte: ");
      // Serial.println((char)rxByte);

      Serial1.println(txData);

      // Serial.print("TX Data: ");
      // Serial.println(txData);
    }
  }
}