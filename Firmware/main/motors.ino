#include "motors.h"

ConfigPWM ConfigMotor = {
    0.4, // 25% of RPM
    0.6, // 50% of RPM
    0.7, // 75% of RPM
    1.0 // 100% of RPM
};

XIN MotorA = {
    new mbed::PwmOut(digitalPinToPinName(6)), // PinAIN1
    new mbed::PwmOut(digitalPinToPinName(9))  // PinAIN2
};

XIN MotorB = {
    new mbed::PwmOut(digitalPinToPinName(3)), // PinBIN1
    new mbed::PwmOut(digitalPinToPinName(5))  // PinBIN2
};

float currentPWM;
void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    currentPWM = ConfigMotor.RPM_50; // Set Default PWM Value
    Serial.println("Motors Initialized!");
}

void balance(float angle, float speed) {
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

void driveMotors(const char* bleBuff) {
    if (!strcmp(bleBuff, "^")) currentPWM = ConfigMotor.RPM_25;
    else if (!strcmp(bleBuff, "v")) currentPWM = ConfigMotor.RPM_50;
    else if (!strcmp(bleBuff, "<")) currentPWM = ConfigMotor.RPM_75;
    else if (!strcmp(bleBuff, ">")) currentPWM = ConfigMotor.RPM_100;

    balance(Angles.Complementary, currentPWM);
}