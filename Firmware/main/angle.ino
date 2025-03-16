#include "angle.h"

/* Constants and Variables */
float k = 0.9;

float prevGyro;
float prevComplementary;

float gx, gy, gz;
float ax, ay, az = 0;

/* Time Variables */
unsigned long t_n, t_n1 = 0; // Current and Previous Time
float dt = 0; // Time Difference

void setupIMU() {
  if (!IMU.begin()) {
    // Serial.println("Failed to Initialize IMU!");
    while (1);
  }

  // Serial.println("IMU Initialized!");
  // Serial.println("Reading Raw Data from Gyroscope and Accelerometer...");
  // Serial.println("Gyroscope (rad/s) | Accelerometer (g)");

  while (az == 0) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);
      prevGyro = atan2(az, ay) * (180 / PI);
      prevGyro -= 90;

      prevComplementary = prevGyro;
    }
  }
}

void getAngles(ANGLES &Angles) {
  float currAccel, currGyro, currComplementary;

  while (!IMU.gyroscopeAvailable());
  IMU.readGyroscope(gx, gy, gz);

  while (!IMU.accelerationAvailable());
  IMU.readAcceleration(ax, ay, az);

  currAccel = atan2(az, ay) * (180 / PI);
  currAccel -= 90;

  // Offset at Low Angles
  if (gx > 0 && gx < 2) gx = 0;
  // Offset at Low Angles
  else if (gx > 0) gx *= 1.10;
  // Account for Negative Angular Velocity Error
  else if (gx < 0) gx *= 1.12;

  if (dt == 0) currGyro = prevGyro + gx * 1 / IMU.gyroscopeSampleRate();
  else currGyro = prevGyro + gx * dt;

  currComplementary = k * (prevComplementary + gx * 1 / IMU.gyroscopeSampleRate()) + (1 - k) * currAccel;

  /* Update Time Variables */
  t_n = millis(); // Current Time in Milliseconds
  dt = (t_n - t_n1) / 1000.0; // Time Difference in Seconds
  t_n1 = t_n; // Assign Current Time to Previous Time

  /* Update Angles */
  Angles.Accelerometer = currAccel;
  Angles.Gyroscope = currGyro;
  Angles.Complementary = currComplementary;

  // Serial.print("Accelerometer: ");
  // Serial.print(Angles.Accelerometer);
  // Serial.print(" | Gyroscope: ");
  // Serial.print(Angles.Gyroscope);
  // Serial.print(" | Complementary: ");
  // Serial.println(Angles.Complementary);

  /* Assign Previous Angles */
  prevGyro = currGyro;
  prevComplementary = currComplementary;
}
