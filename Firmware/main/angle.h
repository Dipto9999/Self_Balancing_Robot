#ifndef ANGLE_H
#define ANGLE_H

#include "Arduino_BMI270_BMM150.h"
#include "serial.h"

struct ANGLES {
    float Accelerometer;
    float Gyroscope;
    float Complementary;
};

/* Constants and Variables */
extern float k;

extern float prev_gyro_angle;
extern float prev_complementary_angle;

extern float gx, gy, gz;
extern float ax, ay, az;

/* Function Prototypes */
void setupIMU();

float editAngleBounds(float angle);
void getAngles(ANGLES &Angles);

#endif