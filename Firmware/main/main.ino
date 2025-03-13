#include <Arduino.h>
#include <mbed.h>

// #include "angle.h"
// #include "controller.h"
// #include "pwm.h"
// #include "ble.h"
#include "serial.h"
#include "gpio.h"

mbed::Ticker TimerTicker;

void timerISR() {
  digitalWrite(PIN_FORWARD_ALERT, !digitalRead(PIN_FORWARD_ALERT));
}

void setup() {
  setupSerial();
  setupGPIO();

  TimerTicker.attach(&timerISR, 0.1f);

  // setupIMU();
  // setupMotors();
  // setupPWM();
  // setupBLE();

  Serial.println("Setup Complete!");
}

// ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
  // Respond to STM32 GPIO Inputs
  // checkForwardAlert();
  checkReverseAlert();

  // Wait for BLE Connection to Override Motors
  // if (rxBLE()) changeDirection(buffBLE);
  // balanceRobot(bleDirection);

  // Send Data
  // serialMsg = String(Angles.Accelerometer, 2) + " " +
  //   String(Angles.Gyroscope, 2) + " " +
  //   String(Angles.Complementary, 2);
  // handleData('A', serialMsg);

  Serial.println("Testing...");
  delay(2000);
}