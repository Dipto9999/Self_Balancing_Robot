#ifndef PWM_H
#define PWM_H

#include "Arduino_BMI270_BMM150.h"

extern int PinAIN1;
extern int PinAIN2;
extern int PinBIN1;
extern int PinBIN2;

extern int val;

/* Function Prototypes */
void setupPWM();
void readPWM(int pin);

#endif