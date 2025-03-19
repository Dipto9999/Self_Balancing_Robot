#include "angle.h"

/* Constants and Variables */
float k;

const float ACCEL_OFFSET = 2.65;
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

      k = 0.9;
    }
  }
}

void getAngles(ANGLES &Angles) {
  float currAccel, currGyro, currComplementary;
  float sampleTime;

  if (!IMU.gyroscopeAvailable()) return;
  IMU.readGyroscope(gx, gy, gz);

  if (!IMU.accelerationAvailable()) return;
  IMU.readAcceleration(ax, ay, az);

  currAccel = atan2(az, ay) * (180 / PI);
  currAccel = (currAccel - 90) + ACCEL_OFFSET;

  // Offset at Low Angles
  if (gx > 0 && gx < 2) gx = 0;
  // Offset at Low Angles
  else if (gx > 0) gx *= 1.10;
  // Account for Negative Angular Velocity Error
  else if (gx < 0) gx *= 1.12;

  if (dt == 0) sampleTime = 1 / IMU.gyroscopeSampleRate();
  else sampleTime = dt;

  currGyro = prevGyro + gx * sampleTime;

  // Case that Robot is Accelerating Forwards
  if ((ax*ax + ay*ay + az*az - 1.0) > 0.04) k = 1;
  else k = 0.9;

  currComplementary = k * (prevComplementary + gx * sampleTime) + (1 - k) * currAccel;

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
