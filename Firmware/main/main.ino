#include <Arduino.h>

#include "angle.h"
#include "controller.h"
#include "ble.h"
#include "serial.h"
#include "gpio.h"
#include "driver.h"

unsigned long lastBLETime = 0;
const unsigned long BLE_INTERVAL = 100;

void setup() {
  setupSerial();
  setupGPIO();

  setupIMU();
  setupBLE();

  setupMotors();

  Serial.println("Setup Complete!");
}

void loop() {
  unsigned long currentMillis = millis();
  // Wait for BLE Connection to Override Motors
  if (currentMillis - lastBLETime >= BLE_INTERVAL) {
    lastBLETime = currentMillis;
    BLE.poll(); // Poll the BLE Device
  }

  if (stmConnected) {
    checkRFID(); // Check RFID Status
    checkForwardAlert();
    checkReverseAlert();
  } else {
    botEnabled = true; // Default to Enabled if STM32 Not Connected
  }

  if (!botEnabled) {
    moveForward(0); // Stop Motors
    return; // Exit Function
  } else if (!isAuthenticated) {
    moveForward(0); // Stop Motors
    if (!pairPrompted) {
      customCharacteristic.writeValue("Pair Device"); // Send Pairing Prompt
      pairPrompted = true; // Set Pairing Prompt Flag
    }
    return; // Exit Function
  }

  getAngles(Angles);
  balanceRobot(bleDirection);

  // updatePID();

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);
}