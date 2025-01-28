#ifndef PWM_H
#define PWM_H

#include "Arduino_BMI270_BMM150.h"
#include "mbed.h"

extern mbed::PwmOut PinAIN1;
extern mbed::PwmOut PinAIN2;
extern mbed::PwmOut PinBIN1;
extern mbed::PwmOut PinBIN2;

/* Function Prototypes */
void setupPWM();
void writePWM(mbed::PwmOut &adc_pin, float dutyCycle);

#endif