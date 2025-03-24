#ifndef DRIVER_H
#define DRIVER_H

#include "controller.h"

void changeDirection(const char* bleBuff);
void moveForward(float dutyCycle);
void moveReverse(float dutyCycle);
void turnLeft(float dutyCycleA, float dutyCycleB);
void turnRight(float dutyCycleA, float dutyCycleB);
float adjustDutyCycle(float u_t, float adjustedPWM);
void drive(float u_t, float errorAngle);

#endif