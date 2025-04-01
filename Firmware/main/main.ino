#include <Arduino.h>

#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"
#include "driver.h"

void setup() {
  setupSerial();
  // setupGPIO();

  setupIMU();
  setupBLE();

  setupMotors();

  Serial.println("Setup Complete!");
}

void updatePID() {
  int parseIndex;
  String paramType, valueStr;
  if (Serial.available() > 0) {
    // Check for PID Parameter Update Command
    String command = Serial.readStringUntil('\n');
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

      if (valueStr != "0" && newValue == 0) {
        Serial.println("Invalid Value!");
        return;
      }

      if (paramType == "k") k = newValue;
      else if (paramType == "set") setpointAngle = newValue;
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

const unsigned long BLE_INTERVAL = 100;
unsigned long lastBLETime = 0;

void loop() {
  // Respond to STM32 GPIO Inputs
  // checkForwardAlert();
  // checkReverseAlert();

  unsigned long currentMillis = millis();
  // Wait for BLE Connection to Override Motors
  if (currentMillis - lastBLETime >= BLE_INTERVAL) {
    lastBLETime = currentMillis;
    BLE.poll(); // Poll the BLE Device
  }

  getAngles(Angles);
  balanceRobot(bleDirection);

  // updatePID();

  // Send Data
  // serialMsg = String(Angles.Accelerometer, 2) + " " +
  //    String(Angles.Gyroscope, 2) + " " +
  //    String(Angles.Complementary, 2);
  // handleData('A', serialMsg);

  // Serial.print("\nSampling Frequency (Hz): ");
  // Serial.println(1.0 / dt);
}