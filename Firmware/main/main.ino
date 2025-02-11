#include "angle.h"
#include "pwm.h"
#include "ble.h"
#include "serial.h"
#include "buttons.h"

void setup() {
    setupSerial();
    setupIMU();
    setupPWM();
    setupBLE();
    setupButtons();
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary


static int pwmIndex = 0;
float pwmValues[] = {
  MotorSpeeds.RPM_25Pct,
  MotorSpeeds.RPM_50Pct,
  MotorSpeeds.RPM_75Pct,
  MotorSpeeds.RPM_100Pct
};
float current_pwm = pwmValues[pwmIndex];

void loop() {
    getAngles(Angles);
    driveMotors(Angles.Complementary, current_pwm);

    // Serial.print("Accel: ");
    // Serial.println(Angles.Accelerometer);

    // Serial.print("Gyro: ");
    // Serial.println(Angles.Gyroscope);

    // Serial.print("Complementary: ");
    // Serial.println(Angles.Complementary);

    if (handleButtons()) {
      pwmIndex = (pwmIndex + 1) % (sizeof(pwmValues) / sizeof(pwmValues[0])); ; // Cycle through RPM Values
      current_pwm = pwmValues[pwmIndex];
    }

  // Send Data
  serialMsg = String(Angles.Accelerometer, 2) + " " +
    String(Angles.Gyroscope, 2) + " " +
    String(Angles.Complementary, 2);
  handleData('A', serialMsg);

  handleBLE();
}