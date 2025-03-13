#include <Arduino.h>
#include <mbed.h>

#include "angle.h"
#include "controller.h"
#include "pwm.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"

mbed::Ticker myTicker;
#define LED_BUILTIN A0

// This is our ISR which toggles the built-in LED.
void timerISR() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
  setupGPIO();
  setupSerial();

  myTicker.attach(&timerISR, 0.1f);

  setupIMU();
  setupMotors();
  setupPWM();
  setupBLE();

  Serial.println("Setup Complete!");
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
  // Respond to STM32 GPIO Inputs
  // checkForwardAlert();
  checkReverseAlert();

  // Wait for BLE Connection to Override Motors
  if (rxBLE()) changeDirection(buffBLE);
  // balanceRobot(bleDirection);

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  Serial.println("Testing...");
}