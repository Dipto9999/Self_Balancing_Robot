#ifndef ANGLE_H
#define ANGLE_H

// #include "Arduino_BMI270_BMM150.h"
#include "Arduino_LSM9DS1.h"
#include "serial.h"

struct ANGLES {
    float Accelerometer;
    float Gyroscope;
    float Complementary;
};
extern ANGLES Angles;

/* Constants and Variables */
extern float k;

extern float initialAngle;

extern const float ACCELEROMETER_OFFSET;
extern const float STANDARD_ACCEL;

extern float prevGyro, prevComplementary;
extern float prevAngle;

extern float gx, gy, gz;
extern float ax, ay, az;

extern unsigned long t_n, t_n1; // Current and Previous Time
extern float dt; // Time Difference

/* Function Prototypes */
void setupIMU();

void getAngles(ANGLES &Angles);

#endif