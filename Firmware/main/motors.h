#ifndef MOTORS_H
#define MOTORS_H

#include "pwm.h"

struct ConfigPWM {
    float RPM_25;
    float RPM_50;
    float RPM_75;
    float RPM_100;
};
extern ConfigPWM ConfigMotor;

extern XIN MotorA;
extern XIN MotorB;

extern float currentPWM;

/* Function Prototypes */
void setupMotors();

void overrideMotors(const char* bleBuff);
void balanceRobot(float angle, float speed);

#endif