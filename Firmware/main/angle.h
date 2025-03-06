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

extern float prevGyro, prevComplementary;

extern float gx, gy, gz;
extern float ax, ay, az;

extern unsigned long t_n, t_n1; // Current and Previous Time
extern float dt; // Time Difference

/* Function Prototypes */
void setupIMU();

// float editAngleBounds(float angle);
void getAngles(ANGLES &Angles);

#endif