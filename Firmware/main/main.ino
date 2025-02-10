#include "angle.h"
#include "pwm.h"
#include "ble.h"
#include "serial.h"


void setup() {
    setupSerial();
    setupIMU();
    setupPWM();
    setupBLE();
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary

void loop() {
    getAngles(Angles);
    driveMotors(Angles.Complementary, 0.75);

    // Serial.print("Accel: ");
    // Serial.println(Angles.Accelerometer);

    // Serial.print("Gyro: ");
    // Serial.println(Angles.Gyroscope);

    // Serial.print("Complementary: ");
    // Serial.println(Angles.Complementary);

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  handleBLE();
}