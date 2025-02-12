#include "angle.h"
#include "motors.h"
#include "ble.h"
#include "serial.h"
// #include "buttons.h"

void setup() {
    setupSerial();
    setupIMU();
    setupMotors();
    setupBLE();
    // setupButtons();
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
    getAngles(Angles);
    balance(Angles.Complementary, currentPWM);

    // Serial.print("Accel: ");
    // Serial.println(Angles.Accelerometer);

    // Serial.print("Gyro: ");
    // Serial.println(Angles.Gyroscope);

    // Serial.print("Complementary: ");
    // Serial.println(Angles.Complementary);

    // if (handleButtons()) {
    //   configIndex = (configIndex + 1) % (sizeof(configVals) / sizeof(configVals[0])); ; // Cycle through RPM Values
    //   currentPWM = configVals[configIndex];
    // }

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);


    // Wait for BLE Connection to Send Data Back to Raspberry Pi
    if (handleBLE()) driveMotors(buffBLE);

    if (connectSuccess) {
      handleData('A', serialMsg);
      // Serial.println(serialMsg);
    }

  // Serial.print("Current PWM: ");
  // Serial.println(currentPWM);
}