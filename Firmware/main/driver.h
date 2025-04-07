#ifndef DRIVER_H
#define DRIVER_H

#include "controller.h"

#define ANGLE_TILT 0.6
#define DEADZONE_PWM 0.055
#define MAX_ERR_ANGLE 0.8

void changeDirection(const char* bleBuff);
float normalizePWM(float u_t, float adjustedPWM);

void moveForward(float dutyCycle);
void moveReverse(float dutyCycle);
void turnLeft(float u_t, float scaleFactor);
void turnRight(float u_t, float scaleFactor);

void drive(float u_t, float errorAngle);

#endif