#ifndef DRIVER_H
#define DRIVER_H

#include "controller.h"

#define ANGLE_TILT 0.5
#define DIRECTION_COUNT 5

void changeDirection(const char* bleBuff);
void moveForward(float dutyCycle);
void moveReverse(float dutyCycle);
void turnLeft(float u_t, float scaleFactor);
void turnRight(float u_t, float scaleFactor);
float normalizePWM(float u_t, float adjustedPWM);
void drive(float u_t, float errorAngle);

#endif