#include "angle.h"
#include "controller.h"
#include "pwm.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"

void setup() {
    setupSerial();
    setupIMU();
    setupMotors();
    setupPWM();
    setupBLE();

    setupGPIO();
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
  // Respond to STM32 GPIO Inputs
  // checkForwardAlert();
  // checkReverseAlert();

  digitalWrite(PIN_FORWARD_ALERT, !digitalRead(PIN_FORWARD_ALERT)); // Toggle LED
  // Serial.println("Hello World!");

  // Wait for BLE Connection to Override Motors
  if (rxBLE()) changeDirection(buffBLE);
  // if (rxBLE()) Serial.println(buffBLE);
  balanceRobot(bleDirection);

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  delay(100);
}