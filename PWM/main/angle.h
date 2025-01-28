#ifndef ANGLE_H
#define ANGLE_H

#include "Arduino_BMI270_BMM150.h"

/* Constants and Variables */
extern float k;

extern float prev_gyro_angle;
extern float prev_complementary_angle;

extern float complementary_angle;
extern float accel_angle;
extern float gyro_angle;

extern float gx, gy, gz;
extern float ax, ay, az;

extern String serialMsg;

/* Function Prototypes */
void handleData(char expectedByte, String txData);
float editAngleBounds(float angle);
void setupSerial();
void setupIMU();
void getAngles(float angles[3]);

#endif