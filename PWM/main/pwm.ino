#include "pwm.h"

// Use the PwmOut Interface to Control Frequency and Duty Cycle.
mbed::PwmOut PinAIN1(digitalPinToPinName(3)); // Pin 3
mbed::PwmOut PinAIN2(digitalPinToPinName(5)); // Pin 5
mbed::PwmOut PinBIN1(digitalPinToPinName(6)); // Pin 6
mbed::PwmOut PinBIN2(digitalPinToPinName(9)); // Pin 9

void setupPWM() {
    PinAIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinAIN1.write(0.5); // Initial Duty Cycle of 50%

    PinAIN2.period_us(20); // PWM Period of 20µs (50kHz)
    PinAIN2.write(0.5); // Initial Duty Cycle of 50%

    PinBIN1.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN1.write(0.5); // Initial Duty Cycle of 50%

    PinBIN2.period_us(20); // PWM Period of 20µs (50kHz)
    PinBIN2.write(0.5); // Initial Duty Cycle of 50%
}

void writePWM(mbed::PwmOut &adc_pin, float dutyCycle) {
    adc_pin.write(dutyCycle);
}
