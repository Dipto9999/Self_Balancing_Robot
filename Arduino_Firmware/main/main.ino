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

  Serial.println("Setup Complete!");
}

void loop() {
  checkEventBLE(); // Check BLE Connection for New Events

  if (stmConnected) {
    checkRFID(); // Check RFID Status
    checkForwardAlert();
    checkReverseAlert();
    checkRedAlert();
  } else {
    botEnabled = true; // Default to Enabled if STM32 Not Connected
  }

  if (!botEnabled) {
    moveForward(0); // Stop Motors
    return; // Exit Function
  } else if (!isAuthenticated) {
    moveForward(0); // Stop Motors
    if (!isPromptedBLE) {
      customCharacteristic.writeValue("Enter Code:"); // Send BLE Prompt
      isPromptedBLE = true; // Set BLE Prompt Flag
    }
    return; // Exit Function
  }

  // Control Loop
  getAngles(Angles);
  balanceRobot(bleDirection);

  // updateParamSerial();

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);
}