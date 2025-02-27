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
float Kd; // Derivative Gain

float setpointAngle; // Reference Value, r_t (Angle = 180°)
float measuredAngle; // Output Value, y_t (Angle)

float errorAngle; // Error Value, e_t = r_t - y_t
float prevErrorAngle; // Previous Error Value, e_(t-1)

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
    Kd = 0.5; // Derivative Gain

    setpointAngle = 180.0; // Reference Value, r_t (Angle = 180°)
    errorAngle = 0.0; // Error Value, e_t = r_t - y_t
    prevErrorAngle = 0.0; // Previous Error Value, e_(t-1)

    currDutyCycle = ConfigMotor.RPM_50; // Set Default PWM Value
}

void balanceRobot(int direction) {
    float dutyCycle;

    getAngles(Angles); // Get Initial Angle Values

    measuredAngle = Angles.Complementary; // Get Measured Angle
    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t

    // Calculate Control Signal
    u_t = (Kp * errorAngle) + (Kd * (errorAngle - prevErrorAngle) / dt);

    // Print Control Values
    Serial.print("Measured Angle: ");
    Serial.println(measuredAngle);
    Serial.print("Error Angle: ");
    Serial.println(errorAngle);
    Serial.print("Prev Error Angle: ");
    Serial.println(prevErrorAngle);

    Serial.print("Time Step: ");
    Serial.println(dt);

    Serial.print("Kp Component: ");
    Serial.println(Kp * errorAngle);

    Serial.print("Kd Component: ");
    Serial.println(Kd * (errorAngle - prevErrorAngle) / dt);
    Serial.print("Control Signal: ");
    Serial.println(u_t);

    // TODO: Convert Control Signal to Power (i.e. PWM Duty Cycle)
    // dutyCycle = constrain(u_t, -1.0, 1.0); // Constrain Control Signal
    dutyCycle = 0.5;

    // TODO: DEPRECATED: Use PID Controller to Balance Robot
    // if (angle >= 215) { // Hard Right (angle ≥ 195)
    //     moveFastDecay(MotorA, CW, ConfigMotor.RPM_100);
    //     moveFastDecay(MotorB, CW, ConfigMotor.RPM_100);
    // } else if (angle <= 135) { // Hard Left (angle ≤ 165)
    //     moveFastDecay(MotorA, CCW, ConfigMotor.RPM_100);
    //     moveFastDecay(MotorB, CCW, ConfigMotor.RPM_100);
    // } else if (angle >= 195) { // Slight Right (angle ≥ 185 && angle < 195)
    //     moveFastDecay(MotorA, CW, ConfigMotor.RPM_75);
    //     moveFastDecay(MotorB, CW, ConfigMotor.RPM_75);
    // } else if (angle <= 155) { // Slight Left (angle ≤ 175 && angle > 165)
    //     moveFastDecay(MotorB, CCW, ConfigMotor.RPM_75);
    //     moveFastDecay(MotorA, CCW, ConfigMotor.RPM_75);
    // } else { // FORWARD Straight (angle ≥ 165 && angle ≤ 195)
    //     moveFastDecay(MotorA, CW, dutyCycle);
    //     moveFastDecay(MotorB, CW, dutyCycle);
    // }

    // TODO: Implement Movement Based on Control Signal (i.e. PWM Duty Cycle) and Direction
    // if (direction == REVERSE) { // REVERSE
        // if (dutyCycle > 0) { // FORWARD
        //     moveFastDecay(MotorA, CW, dutyCycle);
        //     moveFastDecay(MotorB, CW, dutyCycle);
        // } else { // REVERSE
        //     moveFastDecay(MotorA, CCW, -dutyCycle);
        //     moveFastDecay(MotorB, CCW, -dutyCycle);
        // }
    // } else if (direction == FORWARD) { // FORWARD
        // if (dutyCycle > 0) { // FORWARD
        //     moveFastDecay(MotorA, CW, dutyCycle);
        //     moveFastDecay(MotorB, CW, dutyCycle);
        // } else { // REVERSE
        //     moveFastDecay(MotorA, CCW, -dutyCycle);
        //     moveFastDecay(MotorB, CCW, -dutyCycle);
        // }
    // }
    // ...

    prevErrorAngle = errorAngle; // Update Previous Error Value
    return;
}

void changeDirection(const char* bleBuff) {
    // if (!strcmp(bleBuff, "^")) currDirection = FORWARD; // Drive
    // else if (!strcmp(bleBuff, "v")) currDirection = REVERSE; // Reverse
    // else if (!strcmp(bleBuff, "<")) currDirection = LEFT; // Turn Left
    // else if (!strcmp(bleBuff, ">")) currDirection = RIGHT; // Turn Right
    // else if (!strcmp(bleBuff, "X")) currDirection = PARK; // Park

    // TODO: DEPRECATED: Use BLE to Change Direction
    if (!strcmp(bleBuff, "^")) {
        moveFastDecay(MotorA, CW, ConfigMotor.RPM_100);
        moveFastDecay(MotorB, CCW, ConfigMotor.RPM_100);
    }
    else if (!strcmp(bleBuff, "v")) {
        moveFastDecay(MotorA, CW, ConfigMotor.RPM_75);
        moveFastDecay(MotorB, CCW, ConfigMotor.RPM_75);
    }
    else if (!strcmp(bleBuff, "<")) {
        moveFastDecay(MotorA, CCW, ConfigMotor.RPM_50);
        moveFastDecay(MotorB, CW, ConfigMotor.RPM_50);
    }
    else if (!strcmp(bleBuff, ">")) {
        moveFastDecay(MotorA, CCW, ConfigMotor.RPM_25);
        moveFastDecay(MotorB, CW, ConfigMotor.RPM_25);
    }
}