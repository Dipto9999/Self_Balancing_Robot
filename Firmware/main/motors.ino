#include "motors.h"

// At 6V, 289 RPM = 100% Duty Cycle
ConfigPWM ConfigMotor = {
    0.1, // 85 RPM = 25%
    0.375, // 143.5 RPM = 50%
    0.75, // 215 RPM = 75%
    1.0 // 289 RPM (Max) = 100%
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

void balanceRobot(float angle, float speed) {
    float dutyCycle = speed;

    if (angle >= 215) { // Hard Right (angle ≥ 195)
        moveFastDecay(MotorA, FORWARD, ConfigMotor.RPM_100);
        moveFastDecay(MotorB, FORWARD, ConfigMotor.RPM_100);
    } else if (angle <= 135) { // Hard Left (angle ≤ 165)
        moveFastDecay(MotorA, REVERSE, ConfigMotor.RPM_100);
        moveFastDecay(MotorB, REVERSE, ConfigMotor.RPM_100);
    } else if (angle >= 195) { // Slight Right (angle ≥ 185 && angle < 195)
        moveFastDecay(MotorA, FORWARD, ConfigMotor.RPM_75);
        moveFastDecay(MotorB, FORWARD, ConfigMotor.RPM_75);
    } else if (angle <= 155) { // Slight Left (angle ≤ 175 && angle > 165)
        moveFastDecay(MotorB, REVERSE, ConfigMotor.RPM_75);
        moveFastDecay(MotorA, REVERSE, ConfigMotor.RPM_75);
    } else { // Drive Straight (angle ≥ 165 && angle ≤ 195)
        moveFastDecay(MotorA, FORWARD, dutyCycle);
        moveFastDecay(MotorB, FORWARD, dutyCycle);
    }
    return;
}

void overrideMotors(const char* bleBuff) {
    if (!strcmp(bleBuff, "^")) currentPWM = ConfigMotor.RPM_25;
    else if (!strcmp(bleBuff, "v")) currentPWM = ConfigMotor.RPM_50;
    else if (!strcmp(bleBuff, "<")) currentPWM = ConfigMotor.RPM_75;
    else if (!strcmp(bleBuff, ">")) currentPWM = ConfigMotor.RPM_100;
}