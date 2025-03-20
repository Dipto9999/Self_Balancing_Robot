#ifndef BLEMOVEMENT_H
#define BLEMOVEMENT_H

#include "controller.h"

extern int bleDirection;
extern const int VCC;

void changeDirection(const char* bleBuff);
void moveForward(float dutyCycle);
void moveReverse(float dutyCycle);
void turnLeft(float dutyCycleA, float dutyCycleB);
void turnRight(float dutyCycleA, float dutyCycleB);
float calculateNewDutyCycle(float u_t, float pwmPercentage);
void bleMovement_Handle(float u_t, float errorAngle);

#endif