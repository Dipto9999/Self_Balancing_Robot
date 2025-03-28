#include <Arduino.h>

#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"
#include "driver.h"

void setup() {
  setupSerial();
  setupGPIO();

  setupIMU();
  setupBLE();

  setupMotors();

  setupISR();

  // while (!Serial); // Wait for Serial Connection
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

void printControlValues() {
  Serial.print("Measured Angle: ");
  Serial.println(measuredAngle);

  Serial.print("\nError Angle: ");
  Serial.println(errorAngle);
  Serial.print("Prev Error Angle: ");
  Serial.println(prevErrorAngle);
  Serial.print("Accumulated Error: ");
  Serial.println(errorAccumulation);

  Serial.print("\nSampling Frequency (Hz): ");
  Serial.println(1.0 / dt);

  Serial.print("\nKp Component: ");
  Serial.println(Kp * errorAngle);
  Serial.print("Ki Component: ");
  Serial.println(Ki * errorAccumulation);
  Serial.print("Kd Component: ");
  Serial.println(Kd * errorDifference);

  Serial.print("\nControl Signal: ");
  Serial.println(u_t);

  Serial.print("\nDuty Cycle: ");
  Serial.println(currDutyCycle);
}

void printSensorReadings() {
  Serial.print("\nax: ");
  Serial.print(ax);
  Serial.print(" ay: ");
  Serial.print(ay);
  Serial.print(" az: ");
  Serial.print(az);
  Serial.print(" Total Acceleration: ");
  Serial.println(ax*ax + ay*ay + az*az);

  Serial.print("gx: ");
  Serial.print(gx);
  Serial.print(" gy: ");
  Serial.print(gy);
  Serial.print(" gz: ");
  Serial.print(gz);
  Serial.print(" Total Angular Velocity: ");
  Serial.println(gx*gx + gy*gy + gz*gz);
}

void printAngleValues() {
  Serial.print("Accelerometer: ");
  Serial.println(Angles.Accelerometer);
  Serial.print("Gyroscope: ");
  Serial.println(Angles.Gyroscope);
  Serial.print("Complementary: ");
  Serial.println(Angles.Complementary);
}

const unsigned long BLE_INTERVAL = 100;
unsigned long lastBLETime = 0;

void loop() {
  // Respond to STM32 GPIO Inputs
  checkForwardAlert();
  checkReverseAlert();

  unsigned long currentMillis = millis();
  // Wait for BLE Connection to Override Motors
  if (currentMillis - lastBLETime >= BLE_INTERVAL) {
    lastBLETime = currentMillis;
    BLE.poll(); // Poll the BLE Device
  }

  getAngles(Angles);
  // balanceRobot(bleDirection);

  // updatePID();

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
     String(Angles.Gyroscope, 2) + " " +
     String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  // Print Control Values
  // printControlValues();
  // printSensorReadings();

  // Serial.print("\nSampling Frequency (Hz): ");
  // Serial.println(1.0 / dt);

  // Serial.println(ax*ax + ay*ay + az*az);

  // printSensorReadings();
  // printAngleValues();
  // Serial.print("Drifting Condition: ");
  // Serial.println(abs(ax*ax + ay*ay + az*az - STANDARD_ACCEL));
  // Serial.print("k: ");
  // Serial.println(k, 3);
}