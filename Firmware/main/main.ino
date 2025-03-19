#include <Arduino.h>

#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"

void setup() {
  setupSerial();
  setupGPIO();

  setupISR();

  setupIMU();
  setupMotors();
  setupBLE();

  Serial.println("Setup Complete!");
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void printControlValues() {
  Serial.print("Measured Angle: ");
  Serial.println(measuredAngle);

  Serial.print("Error Angle: ");
  Serial.println(errorAngle);
  Serial.print("Prev Error Angle: ");
  Serial.println(prevErrorAngle);
  Serial.print("Accumulated Error: ");
  Serial.println(errorAccumulation);

  Serial.print("Sampling Frequency (Hz): ");
  Serial.println(1.0 / dt);

  Serial.print("Kp Component: ");
  Serial.println(Kp * errorAngle);
  Serial.print("Ki Component: ");
  Serial.println(Ki * errorAccumulation);
  Serial.print("Kd Component: ");
  Serial.println(Kd * errorDifference);

  Serial.print("Control Signal: ");
  Serial.println(u_t);

  Serial.print("Duty Cycle: ");
  Serial.println(currDutyCycle);
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

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  // Print Control Values
  printControlValues();
}