#ifndef PWM_H
#define PWM_H

#include "Arduino_BMI270_BMM150.h"
#include "mbed.h"

enum Dir { FORWARD, REVERSE };

struct MotorPWM {
    float RPM_25Pct;
    float RPM_50Pct;
    float RPM_75Pct;
    float RPM_100Pct;
};
extern MotorPWM MotorSpeeds;

struct XIN {
    mbed::PwmOut* Pin1;
    mbed::PwmOut* Pin2;
};
extern XIN MotorA;
extern XIN MotorB;

extern const int PWM_PeriodUs;

/* Function Prototypes */
void setupPWM();

void moveFastDecay(XIN &motor, Dir dir, float dutyCycle);
void moveSlowDecay(XIN &motor, Dir dir, float dutyCycle);

void driveMotors(float angle, float speed);

#endif