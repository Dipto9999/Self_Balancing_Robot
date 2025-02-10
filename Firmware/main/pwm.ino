#include "pwm.h"

// Use the PwmOut Interface to Control Frequency and Duty Cycle.
mbed::PwmOut PinAIN1(digitalPinToPinName(6)); // Pin 6
mbed::PwmOut PinAIN2(digitalPinToPinName(9)); // Pin 9
mbed::PwmOut PinBIN1(digitalPinToPinName(3)); // Pin 3
mbed::PwmOut PinBIN2(digitalPinToPinName(5)); // Pin 5

void setupPWM() {
    PinAIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinAIN2.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN2.period_us(20); // PWM Period of 20µs (50kHz)

    // moveForwardSlowDecay(PinAIN1, 0.5);
    moveReverseSlowDecay(PinBIN1, 0.5);
}

void writePWM(mbed::PwmOut &PinXIN1, float dutyCycle) {
    PinXIN1.write(dutyCycle);
}

void moveForwardFastDecay(mbed::PwmOut &PinXIN1, float dutyCycle) {
    if (&PinXIN1 == &PinAIN1) {
        PinAIN2.write(0);
    } else if (&PinXIN1 == &PinBIN1) {
        PinBIN2.write(0);
    } else { // Invalid Pin
        return;
    }
    PinXIN1.write(dutyCycle);
}

void moveForwardSlowDecay(mbed::PwmOut &PinXIN1, float dutyCycle) {
    if (&PinXIN1 == &PinAIN1) {
        PinAIN2.write(dutyCycle);
    } else if (&PinXIN1 == &PinBIN1) {
        PinBIN2.write(dutyCycle);
    } else { // Invalid Pin
        return;
    }
    PinXIN1.write(1);
}

void moveReverseFastDecay(mbed::PwmOut &PinXIN1, float dutyCycle) {
    if (&PinXIN1 == &PinAIN1) {
        PinAIN2.write(dutyCycle);
    } else if (&PinXIN1 == &PinBIN1) {
        PinBIN2.write(dutyCycle);
    } else { // Invalid Pin
        return;
    }
    PinXIN1.write(0);
}

void moveReverseSlowDecay(mbed::PwmOut &PinXIN1, float dutyCycle) {
    if (&PinXIN1 == &PinAIN1) {
        PinAIN2.write(1);
    } else if (&PinXIN1 == &PinBIN1) {
        PinBIN2.write(1);
    } else { // Invalid Pin
        return;
    }
    PinXIN1.write(dutyCycle);
}
