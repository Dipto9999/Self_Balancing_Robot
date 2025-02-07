#include "pwm.h"

// Use the PwmOut Interface to Control Frequency and Duty Cycle.
mbed::PwmOut PinAIN1(digitalPinToPinName(3)); // Pin 3
mbed::PwmOut PinAIN2(digitalPinToPinName(5)); // Pin 5
mbed::PwmOut PinBIN1(digitalPinToPinName(6)); // Pin 6
mbed::PwmOut PinBIN2(digitalPinToPinName(9)); // Pin 9

void setupPWM() {
    PinAIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinAIN2.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN2.period_us(20); // PWM Period of 20µs (50kHz)

    moveForwardFastDecay(PinAIN1, 0.5);
    moveReverseFastDecay(PinBIN1, 0.5);
}

void writePWM(mbed::PwmOut &pwm_pin, float dutyCycle) {
    pwm_pin.write(dutyCycle);
}

void moveForwardFastDecay(mbed::PwmOut &pwm_pin, float dutyCycle) {
    if (&pwm_pin == &PinAIN1) {
        pwm_pin.write(dutyCycle);
        PinAIN2.write(0);
    } else if (&pwm_pin == &PinBIN1) {
        pwm_pin.write(dutyCycle);
        PinBIN2.write(0);
    } else { // Invalid Pin
        return;
    }
}

void moveForwardSlowDecay(mbed::PwmOut &pwm_pin, float dutyCycle) {
    pwm_pin.write(1);
    if (&pwm_pin == &PinAIN1) {
        PinAIN2.write(dutyCycle);
    } else if (&pwm_pin == &PinBIN1) {
        PinBIN2.write(dutyCycle);
    } else { // Invalid Pin
        return;
    }
}

void moveReverseFastDecay(mbed::PwmOut &pwm_pin, float dutyCycle) {
    pwm_pin.write(0);
    if (&pwm_pin == &PinAIN1) {
        PinAIN2.write(dutyCycle);
    } else if (&pwm_pin == &PinBIN1) {
        PinBIN2.write(dutyCycle);
    } else { // Invalid Pin
        return;
    }
}

void moveReverseSlowDecay(mbed::PwmOut &pwm_pin, float dutyCycle) {
    pwm_pin.write(1);
    if (&pwm_pin == &PinAIN1) {
        PinAIN2.write(dutyCycle);
    } else if (&pwm_pin == &PinBIN1) {
        PinBIN2.write(dutyCycle);
    } else { // Invalid Pin
        return;
    }
}
