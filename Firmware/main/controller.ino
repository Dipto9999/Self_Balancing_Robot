#include "controller.h"

// At 6V, 289 RPM = 100% Duty Cycle
ConfigPWM ConfigMotor = {
    0.1, // 85 RPM = 25%
    0.375, // 143.5 RPM = 50%
    0.75, // 215 RPM = 75%
    1.0 // 289 RPM (Max) = 100%
};

mbed::Ticker balanceTicker;

const int VCC = 10.8; // 10.8V
const int CONTROL_FREQ = 10; // 10Hz

/* PID Controller Variables */
float setpointAngle; // Reference Value, r_t (Angle = 180°)
float measuredAngle; // Output Value, y_t (Angle)

float errorAngle; // Error Value, e_t = r_t - y_t
float prevErrorAngle; // Previous Error Value, e_(t-1)

float Kp; // Proportional Gain
float Ki; // Integral Gain
float Kd; // Derivative Gain

float errorAccumulation; // Accumulated Error Value, ∑e_t
float errorDifference; // Derivative Error Value, e_t - e_(t-1) / dt
float u_t; // Control Signal

float currDutyCycle; // Current PWM Duty Cycle
int bleDirection; // Current Direction

void balanceRobotISR() {
    // balanceRobot(bleDirection);
    digitalWrite(PIN_FORWARD_ALERT, !digitalRead(PIN_FORWARD_ALERT)); // Toggle LED
}

void setupController() {
    Kp = 0.4; // Proportional Gain
    // Ki = 62.14; // Integral Gain
    Kd = 0; // Derivative Gain

    setpointAngle = 0.0; // Reference Value, r_t (Angle = 180°)
    errorAngle = 0.0; // Error Value, e_t = r_t - y_t
    prevErrorAngle = 0.0; // Previous Error Value, e_(t-1)

    u_t = 0.0; // Control Signal
    errorAccumulation = 0.0; // Accumulated Error Value, ∑e_t
    errorDifference = 0.0; // Derivative Error Value, e_t - e_(t-1) / dt

    bleDirection = FORWARD; // Set Default Direction
    currDutyCycle = ConfigMotor.RPM_50; // Set Default PWM Value

    // Attach ISR to Balance Robot at 10Hz
    //balanceTicker.attach(&balanceRobotISR, 1 / CONTROL_FREQ);
}

void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    setupController(); // Initialize PID Controller
    Serial.println("Motors Initialized!");
}

void balanceRobot(int bleDirection) {
    float dutyCycle;

    getAngles(Angles); // Get Initial Angle Values

    measuredAngle = Angles.Complementary; // Get Measured Angle

    // Ignore Small Angle Values
    if (abs(measuredAngle) < 1.5) {
        return; // Implement BLE Control
    }

    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t
    errorDifference = (errorAngle - prevErrorAngle) / dt; // e_t - e_(t-1) / dt

    // Include Integral Error Accumulation
    // errorAccumulation += (errorAngle * dt); // ∑e_t
    // u_t = (Kp * errorAngle) + (Ki * errorAccumulation) + (Kd * errorDifference);

    // Calculate Control Signal : u_t = Kp * e_t + Ki * ∑e_t + Kd * (e_t - e_(t-1) / dt)
    u_t = (Kp * errorAngle) + (Kd * errorDifference);

    // TODO: Convert Control Signal to Power (i.e. PWM Duty Cycle)
    dutyCycle = abs(u_t) / VCC; // Convert Control Signal to Duty Cycle
    dutyCycle = (dutyCycle > 1) ? 1 : dutyCycle; // Limit Duty Cycle to 100%

    // Print Control Values
    // Serial.print("Measured Angle: ");
    // Serial.println(measuredAngle);

    // Serial.print("Error Angle: ");
    // Serial.println(errorAngle);
    // Serial.print("Prev Error Angle: ");
    // Serial.println(prevErrorAngle);
    // Serial.print("Accumulated Error: ");
    // Serial.println(errorAccumulation);

    // Serial.print("Control Frequency (Hz): ");
    // Serial.println(1.0 / dt);

    // Serial.print("Kp Component: ");
    // Serial.println(Kp * errorAngle);
    // Serial.print("Ki Component: ");
    // Serial.println(Ki * errorAccumulation);
    // Serial.print("Kd Component: ");
    // Serial.println(Kd * errorDifference);

    // Serial.print("Control Signal: ");
    // Serial.println(u_t);

    // Serial.print("Duty Cycle: ");
    // Serial.println(dutyCycle);

    // TODO: DEPRECATED: Use PID Controller to Balance Robot

    // if (u_t > 0) { // FORWARD
    //     moveSlowDecay(MotorA, CW, dutyCycle);
    //     moveSlowDecay(MotorB, CW, dutyCycle);
    // } else { // REVERSE
    //     moveSlowDecay(MotorA, CCW, dutyCycle);
    //     moveSlowDecay(MotorB, CCW, dutyCycle);
    // }

    if (u_t > 0) { // FORWARD
        moveSlowDecay(MotorA, CW, dutyCycle);
        moveSlowDecay(MotorB, CW, dutyCycle);
    } else { // REVERSE
        moveSlowDecay(MotorA, CCW, dutyCycle);
        moveSlowDecay(MotorB, CCW, dutyCycle);
    }

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
    // if (bleDirection == REVERSE) { // REVERSE
        // if (dutyCycle > 0) { // FORWARD
        //     moveFastDecay(MotorA, CW, dutyCycle);
        //     moveFastDecay(MotorB, CW, dutyCycle);
        // } else { // REVERSE
        //     moveFastDecay(MotorA, CCW, -dutyCycle);
        //     moveFastDecay(MotorB, CCW, -dutyCycle);
        // }
    // } else if (bleDirection == FORWARD) { // FORWARD
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
    if (!strcmp(bleBuff, "^") && !forwardAlert) bleDirection = FORWARD; // Drive
    else if (!strcmp(bleBuff, "v") && !reverseAlert) bleDirection = REVERSE; // Reverse
    else if (!strcmp(bleBuff, "<")) bleDirection = LEFT; // Turn Left
    else if (!strcmp(bleBuff, ">")) bleDirection = RIGHT; // Turn Right
    else bleDirection = PARK; // Park

    // TODO: DEPRECATED: Use BLE to Change Direction
    // if (!strcmp(bleBuff, "^")) {
    //     moveFastDecay(MotorA, CW, ConfigMotor.RPM_100);
    //     moveFastDecay(MotorB, CW, ConfigMotor.RPM_100);
    // }
    // else if (!strcmp(bleBuff, "v")) {
    //     moveFastDecay(MotorA, CCW, ConfigMotor.RPM_75);
    //     moveFastDecay(MotorB, CCW, ConfigMotor.RPM_75);
    // }
    // else if (!strcmp(bleBuff, "<")) {
    //     moveFastDecay(MotorA, CW, ConfigMotor.RPM_50);
    //     moveFastDecay(MotorB, CW, ConfigMotor.RPM_50);
    // }
    // else if (!strcmp(bleBuff, ">")) {
    //     moveFastDecay(MotorA, CCW, ConfigMotor.RPM_25);
    //     moveFastDecay(MotorB, CCW, ConfigMotor.RPM_25);
    // }
}

void turnLeft() {
    moveFastDecay(MotorA, CCW, ConfigMotor.RPM_50);
    moveFastDecay(MotorB, CW, ConfigMotor.RPM_50);
}

void turnRight() {
    moveFastDecay(MotorA, CW, ConfigMotor.RPM_50);
    moveFastDecay(MotorB, CCW, ConfigMotor.RPM_50);
}