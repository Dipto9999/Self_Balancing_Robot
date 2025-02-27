#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
// #include "buttons.h"

void setup() {
    setupSerial();
    setupIMU();
    setupMotors();
    setupBLE();

    // Test With Buttons
    // setupButtons();
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
  // Test With Buttons
  // if (handleButtons()) {
    // currDutyCycle = ...
  // }

  // Wait for BLE Connection to Override Motors
  if (rxBLE()) changeDirection(buffBLE);
  balanceRobot(currDirection);

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);
}