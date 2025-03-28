#include "driver.h"

#define ERROR_ANGLE_MAX 3
#define PWM_DRIVE_ADJUSTMENT 0.05

void changeDirection(const char* bleBuff) {
    // if (!strcmp(bleBuff, "^") && !forwardAlert) bleDirection = FORWARD; // Drive
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

void turnLeft(float dutyCycleA, float dutyCycleB) {
    moveSlowDecay(MotorA, CCW, dutyCycleA);
    moveSlowDecay(MotorB, CW, dutyCycleB);
}

void turnRight(float dutyCycleA, float dutyCycleB) {
    moveSlowDecay(MotorA, CW, dutyCycleA);
    moveSlowDecay(MotorB, CCW, dutyCycleB);
}

float adjustDutyCycle(float u_t, float adjustedPWM) {
    float dutyCycle = (abs(u_t) - 0.055) / (VCC - 0.055) *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : dutyCycle; // Limit Duty Cycle to 100%
}

void drive(float u_t, float errorAngle) {
    float dutyCycleA = adjustDutyCycle(u_t, 0);

    switch (bleDirection) {
        case FORWARD:
            moveForward(0.5);
            break;
        case REVERSE:
            moveReverse(0.5);
            break;
        case LEFT:
            turnLeft(0.75, 0.5);
            break;
        case RIGHT:
            turnRight(0.75, 0.5);
            break;
        case IDLE:
            break;
        default:
            // Handle unexpected values, if necessary
            break;
    }
    currDutyCycle = dutyCycleA; // Update Current Duty Cycle

    if (u_t > 0) moveForward(dutyCycleA);
    else moveReverse(dutyCycleA);
}
