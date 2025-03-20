#include "ble_movement.h"

#define ERROR_ANGLE_MAXIMUM 3
#define PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT 0.05

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

float calculateNewDutyCycle(float u_t, float pwmPercentage) {
    float dutyCycle = abs(u_t) / VCC *  (1 + pwmPercentage); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 0.75) ? 0.75 : dutyCycle; // Limit Duty Cycle to 100%
}

void bleMovement_Handle(float u_t, float errorAngle) {
    float dutyCycleA = calculateNewDutyCycle(u_t, 0);

    // Serial.print("BLE Direction: ");
    // Serial.println(bleDirection);

    switch (bleDirection)
    {
        case FORWARD:
            // if errorAngle > 0, then should move FORWARD to compensate
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM) {
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
            }
            break;
        case REVERSE:
            if (errorAngle < 0 && errorAngle > -ERROR_ANGLE_MAXIMUM) {
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
            }
            break;
        case LEFT:
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM) {
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
                turnLeft(dutyCycleA, dutyCycleA);
                return;
            }
            break;
        case RIGHT:
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM) {
                dutyCycleA = calculateNewDutyCycle(u_t, PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
                turnRight(dutyCycleA, dutyCycleA);
                return;
            }
            break;
        case IDLE:
            break;
        default:
            // Handle unexpected values, if necessary
            break;
    }
    // Serial.print("Duty Cycle A: ");
    // Serial.println(dutyCycleA);

    if (u_t > 0) moveForward(dutyCycleA);
    else moveReverse(dutyCycleA);
}
