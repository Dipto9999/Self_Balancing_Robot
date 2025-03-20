#include <Arduino.h>

#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"
#include "ble_movement.h"

void setup() {
  setupSerial();
  setupGPIO();

  setupISR();

  setupIMU();
  setupMotors();
  setupBLE();

  Serial.println("Setup Complete!");
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

void loop() {
  // Respond to STM32 GPIO Inputs
  checkForwardAlert();
  checkReverseAlert();

  // Wait for BLE Connection to Override Motors
  if (rxBLE()) changeDirection(buffBLE);

  getAngles(Angles);
  // balanceRobot(bleDirection);

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  // Print Control Values
  // printControlValues();
  // printSensorReadings();
}