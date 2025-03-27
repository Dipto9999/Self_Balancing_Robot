#include "controller.h"

// At 6V, 289 RPM = 100% Duty Cycle
ConfigPWM ConfigMotor = {
    0.1, // 85 RPM = 25%
    0.375, // 143.5 RPM = 50%
    0.75, // 215 RPM = 75%
    1.0 // 289 RPM (Max) = 100%
};

// const int VCC = 10.8; // 10.8 V
const int VCC = 10.4; // 10.4 V

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

void setupController() {
    // Kp = 0.5; // Proportional Gain
    // Ki = 0; // Integral Gain
    // Kd = 0.07; // Derivative Gain

    Kp = 0.08; // Proportional Gain
    Ki = 3.5; // Integral Gain
    Kd = 0.04; // Derivative Gain

    // setpointAngle = 0.0; // Reference Value, r_t (Angle = 180°)
    setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)
    errorAngle = 0.0; // Error Value, e_t = r_t - y_t
    prevErrorAngle = 0.0; // Previous Error Value, e_(t-1)

    u_t = 0.0; // Control Signal
    errorAccumulation = 0.0; // Accumulated Error Value, ∑e_t
    errorDifference = 0.0; // Derivative Error Value, e_t - e_(t-1) / dt

    bleDirection = IDLE; // Set Default Direction
    currDutyCycle = ConfigMotor.RPM_50; // Set Default PWM Value
}

void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    setupController(); // Initialize PID Controller
    Serial.println("Motors Initialized!");
}

void balanceRobot(int bleDirection) {
    // TODO: Measure Angles in Main Loop
    // getAngles(Angles); // Get Initial Angle Values

    // Get Measured Angle
    measuredAngle = Angles.Complementary; // Complementary Filter

    // TODO: Ignore Small Angle Values
    // if (abs(measuredAngle) < 1) {
    //     return; // Implement BLE Control
    // }

    // if (driftingCondition) {
    //     setpointAngle = -0.5 * measuredAngle; // Invert Setpoint Angle
    // } else {
    //     setpointAngle = 0; // Setpoint Angle = 0°
    // }

    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t
    errorDifference = (errorAngle - prevErrorAngle) / dt; // e_t - e_(t-1) / dt

    if (prevAngle * measuredAngle < 0) {
        // driftingCondition = false; // Reset Drifting Condition
        errorAccumulation = errorAngle * dt; // Reset Accumulated Error Value ∑e_t
    } else if ((round(measuredAngle) == setpointAngle) && (abs(setpointAngle - prevAngle) < 1)) {
        errorAccumulation = 0; // Reset Accumulated Error Value ∑e_t
    } else {
        errorAccumulation += (errorAngle * dt); // Include Integral Error Accumulation ∑e_t
    }

    // Calculate Control Signal : u_t = Kp * e_t + Ki * ∑e_t + Kd * (e_t - e_(t-1) / dt)
    // u_t = (Kp * errorAngle) + (Kd * errorDifference);
    u_t = (Kp * errorAngle) + (Ki * errorAccumulation) + (Kd * errorDifference);

    // Serial.print("u_t: ");
    // Serial.println(u_t);

    // Serial.print("Error Angle: ");
    // Serial.println(errorAngle);

    drive(u_t, errorAngle);

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

    prevErrorAngle = errorAngle; // Update Previous Error Value
    return;
}
