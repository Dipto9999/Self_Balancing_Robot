#include "driver.h"

#define ERROR_ANGLE_MAX 3
#define PWM_DRIVE_ADJUSTMENT 0.05

int balanceCounter = 0; // Counter for Balance Control Loop
void changeDirection(const char* bleBuff) {
    // if (!strcmp(bleBuff, "^") && !forwardAlert) bleDirection = FORWARD; // Drive

    balanceCounter = 0; // Reset Balance Counter
    if (!strcmp(bleBuff, "^")) bleDirection = FORWARD; // Drive
    // else if (!strcmp(bleBuff, "v") && !reverseAlert) bleDirection = REVERSE; // Reverse
    else if (!strcmp(bleBuff, "v")) bleDirection = REVERSE; // Reverse
    else if (!strcmp(bleBuff, "<")) bleDirection = LEFT; // Turn Left
    else if (!strcmp(bleBuff, ">")) bleDirection = RIGHT; // Turn Right
    else bleDirection = IDLE; // IDLE
}

void moveForward(float dutyCycle) {
    moveSlowDecay(MotorA, CW, dutyCycle);
    moveSlowDecay(MotorB, CW, dutyCycle);
}

void moveReverse(float dutyCycle) {
    moveSlowDecay(MotorA, CCW, dutyCycle);
    moveSlowDecay(MotorB, CCW, dutyCycle);
}

void turnLeft(float dutyCycle, float dutyCycleB) {
    moveSlowDecay(MotorA, CCW, dutyCycle);
    moveSlowDecay(MotorB, CW, dutyCycleB);
}

void turnRight(float dutyCycle, float dutyCycleB) {
    moveSlowDecay(MotorA, CW, dutyCycle);
    moveSlowDecay(MotorB, CCW, dutyCycleB);
}

float normalizePWM(float u_t, float adjustedPWM) {
    float dutyCycle = (abs(u_t) - 0.055) / (VCC - 0.055) *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : dutyCycle; // Limit Duty Cycle to 100%
}

void drive(float u_t, float errorAngle) {
    float dutyCycle = normalizePWM(u_t, 0);

    switch (bleDirection) {
        case FORWARD:
            // setpointAngle = SETPOINT_0 + 3; // Reference Value, r_t (Angle = 180°)

            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
            break;
        case REVERSE:
            setpointAngle = SETPOINT_0 - 3; // Reference Value, r_t (Angle = 180°)

            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
        break;
        case LEFT:
            // setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)

            if (balanceCounter++ == 4) {
                balanceCounter = 0; // Reset Balance Counter
                turnLeft(dutyCycle, 0);
            } else {
                if (u_t > 0) moveForward(dutyCycle);
                else moveReverse(dutyCycle);
            }
        break;
        case RIGHT:
            // setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)

            if (balanceCounter++ == 4) {
                balanceCounter = 0; // Reset Balance Counter
                turnRight(0, dutyCycle);
            } else {
                if (u_t > 0) moveForward(dutyCycle);
                else moveReverse(dutyCycle);
            }
        break;
        default:
            // setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)

            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
        break;
    }
    currDutyCycle = dutyCycle; // Update Current Duty Cycle
}
