#ifndef PWM_H
#define PWM_H

#include "Arduino_BMI270_BMM150.h"
#include "mbed.h"
#include "serial.h"

enum Dir { FORWARD, REVERSE };

struct XIN {
    mbed::PwmOut* Pin1;
    mbed::PwmOut* Pin2;
};

extern const int PWM_PeriodUs;

/* Function Prototypes */
void setupPWM();

void moveFastDecay(XIN &motor, Dir dir, float dutyCycle);
void moveSlowDecay(XIN &motor, Dir dir, float dutyCycle);

#endif