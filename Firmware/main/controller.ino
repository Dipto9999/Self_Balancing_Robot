#include "controller.h"

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
    // Kp = 0.2; // Proportional Gain
    // Ki = 6.5; // Integral Gain
    // Kd = 0.08; // Derivative Gain

    Kp = 0.7;
    Ki = 11.25;
    Kd = 0.055;

    setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)
    errorAngle = 0.0; // Error Value, e_t = r_t - y_t
    prevErrorAngle = 0.0; // Previous Error Value, e_(t-1)

    u_t = 0.0; // Control Signal
    errorAccumulation = 0.0; // Accumulated Error Value, ∑e_t
    errorDifference = 0.0; // Derivative Error Value, e_t - e_(t-1) / dt

    bleDirection = IDLE; // Set Default Direction
    currDutyCycle = 0.5; // Set Default PWM Value

    pinMode(DISABLE_INTEGRAL_BUTTON, INPUT_PULLUP);
}

void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    setupController(); // Initialize PID Controller
    Serial.println("Motors Initialized!");
}

void balanceRobot(int bleDirection) {
    // Get Measured Angle
    measuredAngle = Angles.Complementary; // Complementary Filter
    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t
    errorDifference = (errorAngle - prevErrorAngle) / dt; // e_t - e_(t-1) / dt
    errorAccumulation += (errorAngle * dt);
    /*
    if ((prevErrorAngle * errorAngle) < 0) {
        errorAccumulation = errorAngle * dt; // Reset Accumulated Error Value ∑e_t
    } else if (abs(errorAngle) < 0.3) {
        errorAccumulation = errorAngle * dt; // Reset Accumulated Error Value ∑e_t
    } else {
        errorAccumulation += (errorAngle * dt); // Include Integral Error Accumulation ∑e_t
    }
    */
    /*
    if (errorAccumulation > MAX_ERROR_ACCUMULATION)
    {
        errorAccumulation = MAX_ERROR_ACCUMULATION; // Limit Accumulated Error Value ∑e_t
    }
    else if (errorAccumulation < -MAX_ERROR_ACCUMULATION)
    {
        errorAccumulation = -MAX_ERROR_ACCUMULATION; // Limit Accumulated Error Value ∑e_t
    }
    */

    if (digitalRead(DISABLE_INTEGRAL_BUTTON) == LOW) {
        errorAccumulation = 0; // Reset Accumulated Error Value ∑e_t
    }
        
    // Serial.print("Error Angle: ");
    // Serial.println(errorAngle, 3);
    // Serial.print(" Previous Error Angle: ");
    // Serial.println(prevErrorAngle, 3);
    // Serial.print(" Error Accumulation: ");
    // Serial.println(errorAccumulation, 3);
    // Serial.println("");

    // if (((prevAngle - measuredAngle) - setpointAngle) < 0) {
    //     errorAccumulation = errorAngle * dt; // Reset Accumulated Error Value ∑e_t
    // } else if ((abs(setpointAngle - measuredAngle) < 0.3) && (abs(setpointAngle - prevAngle) < 1)) {
    //     errorAccumulation = 0; // Reset Accumulated Error Value ∑e_t
    // } else {
    //     errorAccumulation += (errorAngle * dt); // Include Integral Error Accumulation ∑e_t
    // }

    // Calculate Control Signal : u_t = Kp * e_t + Ki * ∑e_t + Kd * (e_t - e_(t-1) / dt)
    // u_t = (Kp * errorAngle) + (Kd * errorDifference);
    u_t = (Kp * errorAngle) + (Ki * errorAccumulation) + (Kd * errorDifference);

    drive(u_t, errorAngle);

    prevErrorAngle = errorAngle; // Update Previous Error Value
    return;
}
