#include "angle.h"

/* Constants and Variables */
float k = 0.6;

float prev_gyro_angle;
float prev_complementary_angle;

float complementary_angle = 0;
float accel_angle = 0;
float gyro_angle = 0;

float gx, gy, gz;
float ax, ay, az = 0;

String serialMsg = "";

void handleData(char expectedByte, String txData) {
  // Send Data When Rx Byte Received
  if (Serial.available() > 0) {
    // Read Incoming Byte
    int rxByte = Serial.read();
    // Receive Byte rxByte to Send Angle Data
    if (rxByte == expectedByte) {
      Serial.println(txData);
    }
  }
}

float editAngleBounds(float angle) {
  if (angle > 360) angle -= 360;
  else if (angle < 0) angle += 360;
  return angle;
}

void setupSerial() {
  Serial.begin(115200);
}

void setupIMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("IMU initialized!");
  Serial.println("Reading raw data from gyroscope and accelerometer...");
  Serial.println("Gyroscope (rad/s) | Accelerometer (g)");

  while (az == 0) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);
      prev_gyro_angle = atan2(az, ay) * (180 / PI);
      prev_gyro_angle += 90;
      prev_gyro_angle = editAngleBounds(prev_gyro_angle);

      prev_complementary_angle = prev_gyro_angle;
    }
  }
}

void getAngles(float angles[3]) {
  while (!IMU.gyroscopeAvailable());
  IMU.readGyroscope(gx, gy, gz);

  while (!IMU.accelerationAvailable());
  IMU.readAcceleration(ax, ay, az);

  accel_angle = atan2(az, ay) * (180 / PI);
  accel_angle += 90;
  accel_angle = editAngleBounds(accel_angle);

  // Offset at Low Angles
  if (gx > 0 && gx < 2) gx = 0;
  // Offset at Low Angles
  else if (gx > 0) gx *= 1.10;
  // Account for Negative Angular Velocity Error
  else if (gx < 0) gx *= 1.12;

  // gyro_angle = prev_accel_angle + gx * 1 / IMU.gyroscopeSampleRate();
  gyro_angle = prev_gyro_angle + gx * 1 / IMU.gyroscopeSampleRate();
  gyro_angle = editAngleBounds(gyro_angle);

  complementary_angle = k * (prev_complementary_angle + gx * 1 / IMU.gyroscopeSampleRate()) + (1 - k) * accel_angle;
  complementary_angle = editAngleBounds(complementary_angle);

  // Send Data
  // serialMsg = String(accel_angle, 2) + " " +
  //   String(gyro_angle, 2) + " " +
  //   String(complementary_angle, 2);
  // handleData('A', serialMsg);

  // prev_gyro_angle = gyro_angle;
  // prev_complementary_angle = complementary_angle;

  angles[0] = accel_angle;
  angles[1] = gyro_angle;
  angles[2] = complementary_angle;
}
