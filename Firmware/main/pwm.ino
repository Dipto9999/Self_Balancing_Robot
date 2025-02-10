#include "pwm.h"

const int PWM_PeriodUs = 20; // 20µs Period (50kHz)

XIN MotorA = {
    new mbed::PwmOut(digitalPinToPinName(6)), // PinAIN1
    new mbed::PwmOut(digitalPinToPinName(9))  // PinAIN2
};

XIN MotorB = {
    new mbed::PwmOut(digitalPinToPinName(3)), // PinBIN1
    new mbed::PwmOut(digitalPinToPinName(5))  // PinBIN2
};

void setupPWM() {
    MotorA.Pin1->period_us(PWM_PeriodUs);
    MotorA.Pin2->period_us(PWM_PeriodUs);
    MotorB.Pin1->period_us(PWM_PeriodUs);
    MotorB.Pin2->period_us(PWM_PeriodUs);
}

/*
 * Fast Decay: Hold Primary Input at PWM Duty Cycle, Secondary Input Low.
 */
void moveFastDecay(XIN &motor, Dir dir, float dutyCycle) {
    if (dir == FORWARD) {
        motor.Pin1->write(dutyCycle);
        motor.Pin2->write(0);
    } else if (dir == REVERSE) {
        motor.Pin1->write(0);
        motor.Pin2->write(dutyCycle);
    }
}

/*
 * Slow Decay: Hold Primary Input High, Secondary Input at PWM Duty Cycle.
 */
void moveSlowDecay(XIN &motor, Dir dir, float dutyCycle) {
    if (dir == FORWARD) {
        motor.Pin1->write(1);
        motor.Pin2->write(dutyCycle);
    } else if (dir == REVERSE) {
        motor.Pin1->write(dutyCycle);
        motor.Pin2->write(1);
    }
}

void driveMotors(float angle, float speed) {
    float dutyCycle = speed;

    if (angle >= 195) { // Hard Right (angle ≥ 195)
        moveFastDecay(MotorA, FORWARD, dutyCycle); // Hold Motor A High
        moveFastDecay(MotorB, REVERSE, 1); // Drive Motor B in Reverse
    } else if (angle <= 165) { // Hard Left (angle ≤ 165)
        moveFastDecay(MotorB, FORWARD, dutyCycle); // Hold Motor B High
        moveFastDecay(MotorA, REVERSE, 1); // Drive Motor A in Reverse
    } else if (angle >= 185) { // Slight Right (angle ≥ 185 && angle < 195)
        moveFastDecay(MotorA, FORWARD, dutyCycle); // Hold Motor A High
        moveFastDecay(MotorB, REVERSE, 0.5); // Drive Motor A in Reverse
    } else if (angle <= 175) { // Slight Left (angle ≤ 175 && angle > 165)
        moveFastDecay(MotorB, FORWARD, dutyCycle); // Hold Motor B High
        moveFastDecay(MotorA, REVERSE, 0.5); // Drive Motor A in Reverse
    } else { // Drive Straight (angle ≥ 175 && angle ≤ 185)
        moveFastDecay(MotorA, FORWARD, dutyCycle); // Hold Motor A High
        moveFastDecay(MotorB, FORWARD, dutyCycle); // Hold Motor B High
    }
    return;
}