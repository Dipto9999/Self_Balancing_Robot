#ifndef DRIVER_H
#define DRIVER_H

#include "controller.h"

#define ANGLE_TILT 0.6
#define DEADZONE_PWM 0.055
#define MAX_ERR_ANGLE 0.8

extern float startTime, currTime;
extern int directionCount; // Count Direction Changes

extern float normalizePWM(float u_t, float adjustedPWM);

extern void changeDirection(const char* bleBuff);
extern void moveForward(float dutyCycle);
extern void moveReverse(float dutyCycle);
extern void turnLeft(float u_t, float scaleFactor);
extern void turnRight(float u_t, float scaleFactor);

extern void drive(float u_t, float errorAngle);

#endif