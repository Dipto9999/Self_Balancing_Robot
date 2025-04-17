#include "angle.h"

float k; // Complementary Filter Constant

const float ACCELEROMETER_OFFSET = 0;

const float STANDARD_ACCEL = 0.95;

float prevGyro, prevComplementary;
float prevAngle = 0;

float gx, gy, gz;
float ax, ay, az = 0;

const long MAX_CALIBRATION = 50;
unsigned long calibration_count = 0;

/* Time Variables */
unsigned long t_n, t_n1 = 0; // Current and Previous Time
float dt = 0; // Time Difference

float initialAngle;

void setupIMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to Initialize IMU!");
    while (1);
  }

  k = 0.95;
  while (az == 0) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);
      prevGyro = atan2(az, ay) * (180 / PI);
      prevGyro -= 90;

      initialAngle = prevGyro;
      prevComplementary = prevGyro;
    }
  }
}

ANGLES Angles = {0, 0, 0}; // Accelerometer, Gyroscope, Complementary
void getAngles(ANGLES &Angles) {
  float currAccel, currGyro, currComplementary;
  float sampleTime;

  if (!IMU.gyroscopeAvailable()) return;
  IMU.readGyroscope(gx, gy, gz);

  if (!IMU.accelerationAvailable()) return;
  IMU.readAcceleration(ax, ay, az);

  currAccel = atan2(az, ay) * (180 / PI);
  currAccel = (currAccel - 90) + ACCELEROMETER_OFFSET;

  sampleTime = 1.0 / IMU.gyroscopeSampleRate();

  currGyro = prevGyro + gx * sampleTime;

  prevAngle = prevComplementary;
  currComplementary = k * (prevComplementary + gx * sampleTime) + (1 - k) * currAccel;

  /* Update Time Variables */
  t_n = millis(); // Current Time in Milliseconds
  dt = (t_n - t_n1) / 1000.0; // Time Difference in Seconds
  t_n1 = t_n; // Assign Current Time to Previous Time

  /* Update Angles */
  Angles.Accelerometer = currAccel;
  Angles.Gyroscope = currGyro;
  Angles.Complementary = currComplementary;

  /* Assign Previous Angles */
  prevGyro = currGyro;
  prevComplementary = currComplementary;
}
