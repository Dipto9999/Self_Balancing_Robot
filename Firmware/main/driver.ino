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
    float dutyCycle = abs(u_t) / VCC *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle < 0.055) ? 0.055 : ((dutyCycle > 1) ? 1 : dutyCycle); // Limit Duty Cycle to 100%
}

void drive(float u_t, float errorAngle) {
    float dutyCycleA = adjustDutyCycle(u_t, 0);

    // Serial.print("BLE Direction: ");
    // Serial.println(bleDirection);

    switch (bleDirection) {
        case FORWARD:
            setpointAngle = 0.5; // Reference Value, r_t (Angle = 180°)
            // if errorAngle > 0, then should move FORWARD to compensate
            // if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAX) {
            //     dutyCycleA = adjustDutyCycle(u_t, -PWM_DRIVE_ADJUSTMENT);
            // }
            // else if (errorAngle > -ERROR_ANGLE_MAX && errorAngle < 0)
            // {
            //     dutyCycleA = adjustDutyCycle(u_t, PWM_DRIVE_ADJUSTMENT);
            // }

            break;
        case REVERSE:
            setpointAngle = -0.5; // Reference Value, r_t (Angle = 180°)
            // if (errorAngle < 0 && errorAngle > -ERROR_ANGLE_MAX) {
            //     dutyCycleA = adjustDutyCycle(u_t, -PWM_DRIVE_ADJUSTMENT);
            // }
            // else if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAX)
            // {
            //     dutyCycleA = adjustDutyCycle(u_t, PWM_DRIVE_ADJUSTMENT);
            // }

            break;
        case LEFT:
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAX) {
                // dutyCycleA = adjustDutyCycle(u_t, -PWM_DRIVE_ADJUSTMENT / 2);
                turnLeft(dutyCycleA, dutyCycleA);
                return;
            }
            break;
        case RIGHT:
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAX) {
                // dutyCycleA = adjustDutyCycle(u_t, PWM_DRIVE_ADJUSTMENT / 2);
                turnRight(dutyCycleA, dutyCycleA);
                return;
            }
            break;
        case IDLE:
            setpointAngle = 0.75; // Reference Value, r_t (Angle = 180°)
            break;
        default:
            // Handle unexpected values, if necessary
            break;
    }
    // Serial.print("Duty Cycle A: ");
    // Serial.println(dutyCycleA);

    currDutyCycle = dutyCycleA; // Update Current Duty Cycle

    if (u_t > 0) moveForward(dutyCycleA);
    else moveReverse(dutyCycleA);
}
