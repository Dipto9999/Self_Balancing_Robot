#include "pwm.h"

const int PWM_PeriodUs = 20; // 20µs Period (50kHz)

void setupPWM() {
    MotorA.Pin1->period_us(PWM_PeriodUs);
    MotorA.Pin2->period_us(PWM_PeriodUs);
    MotorB.Pin1->period_us(PWM_PeriodUs);
    MotorB.Pin2->period_us(PWM_PeriodUs);

    Serial.println("PWM Initialized!");
}

/*
 * Fast Decay: Hold Primary Input at PWM Duty Cycle, Secondary Input Low.
 */
void moveFastDecay(XIN &motor, DirPWM dir, float dutyCycle) {
    if (dir == CW) {
        motor.Pin1->write(dutyCycle);
        motor.Pin2->write(0);
    } else if (dir == CCW) {
        motor.Pin1->write(0);
        motor.Pin2->write(dutyCycle);
    }
}

/*
 * Slow Decay: Hold Primary Input High, Secondary Input at PWM Duty Cycle.
 */
void moveSlowDecay(XIN &motor, DirPWM dir, float dutyCycle) {
    if (dir == CW) {
        motor.Pin1->write(1);
        motor.Pin2->write(1-dutyCycle);
    } else if (dir == CCW) {
        motor.Pin1->write(1-dutyCycle);
        motor.Pin2->write(1);
    }
}