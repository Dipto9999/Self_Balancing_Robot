#include "controller.h"

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

/* PID Controller Variables */
float Kp; // Proportional Gain
float Ki; // Integral Gain
float Kd; // Derivative Gain

float setpointAngle; // Reference Value, r_t (Angle = 180°)
float measuredAngle; // Output Value, y_t (Angle)

float errorAngle; // Error Value, e_t = r_t - y_t
float prevErrorAngle; // Previous Error Value, e_(t-1)

float integralAngle; // Integral Value, ∑(e_t * dt)
float derivativeAngle; // Derivative Value, (e_t - e_(t-1)) / dt

float u_t; // Control Signal

float currDutyCycle; // Current PWM Duty Cycle
int currDirection; // Current Direction

void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    setupController(); // Initialize PID Controller
    Serial.println("Motors Initialized!");
}

void setupController() {
    Kp = 0.5; // Proportional Gain
    Ki = 0.5; // Integral Gain
    Kd = 0.0; // Derivative Gain

    setpointAngle = 180.0; // Reference Value, r_t (Angle = 180°)
    integralAngle = 0.0;

    currDutyCycle = ConfigMotor.RPM_50; // Set Default PWM Value
}

void balanceRobot(int direction) {
    float dutyCycle;

    getAngles(Angles); // Get Initial Angle Values
    measuredAngle = Angles.Complementary; // Get Complementary Filter Angle

    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t
    integralAngle += errorAngle * dt; // Calculate Integral Value
    derivativeAngle = (errorAngle - prevErrorAngle) / dt; // Calculate Derivative Value

    // Calculate Control Signal
    u_t = (Kp * errorAngle) + (Ki * integralAngle) + (Kd * derivativeAngle);
    // TODO: Convert Control Signal to PWM Duty Cycle
    dutyCycle = constrain(u_t, -1.0, 1.0); // Constrain Control Signal

    if (dutyCycle > 0) { // Forward
        moveFastDecay(MotorA, FORWARD, dutyCycle);
        moveFastDecay(MotorB, FORWARD, dutyCycle);
    } else { // Reverse
        moveFastDecay(MotorA, REVERSE, -dutyCycle);
        moveFastDecay(MotorB, REVERSE, -dutyCycle);
    }

    // TODO: DEPRECATED: Use PID Controller to Balance Robot
    // if (angle >= 215) { // Hard Right (angle ≥ 195)
    //     moveFastDecay(MotorA, FORWARD, ConfigMotor.RPM_100);
    //     moveFastDecay(MotorB, FORWARD, ConfigMotor.RPM_100);
    // } else if (angle <= 135) { // Hard Left (angle ≤ 165)
    //     moveFastDecay(MotorA, REVERSE, ConfigMotor.RPM_100);
    //     moveFastDecay(MotorB, REVERSE, ConfigMotor.RPM_100);
    // } else if (angle >= 195) { // Slight Right (angle ≥ 185 && angle < 195)
    //     moveFastDecay(MotorA, FORWARD, ConfigMotor.RPM_75);
    //     moveFastDecay(MotorB, FORWARD, ConfigMotor.RPM_75);
    // } else if (angle <= 155) { // Slight Left (angle ≤ 175 && angle > 165)
    //     moveFastDecay(MotorB, REVERSE, ConfigMotor.RPM_75);
    //     moveFastDecay(MotorA, REVERSE, ConfigMotor.RPM_75);
    // } else { // Drive Straight (angle ≥ 165 && angle ≤ 195)
    //     moveFastDecay(MotorA, FORWARD, dutyCycle);
    //     moveFastDecay(MotorB, FORWARD, dutyCycle);
    // }
    return;
}

void changeDirection(const char* bleBuff) {
    if (!strcmp(bleBuff, "^")) currDirection = DRIVE;
    else if (!strcmp(bleBuff, "v")) currDirection = BACK;
    else if (!strcmp(bleBuff, "<")) currDirection = LEFT;
    else if (!strcmp(bleBuff, ">")) currDirection = RIGHT;
    // else if (!strcmp(bleBuff, "X")) currDirection = STOP;
}