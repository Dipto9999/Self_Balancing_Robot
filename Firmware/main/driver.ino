#include "driver.h"

float startTime, currTime; // Time Variables for Control Loop
int directionCount; // Direction Count for BLE Commands

void changeDirection(const char* bleBuff) {
    if (!strcmp(bleBuff, "^") && (!forwardAlert)) {
        setpointAngle = SETPOINT_0 - ANGLE_TILT;
        bleDirection = FORWARD; // Drive Forward
    } else if (!strcmp(bleBuff, "v") && (!reverseAlert)) {
        setpointAngle = SETPOINT_0 + ANGLE_TILT;
        bleDirection = REVERSE; // Drive Reverse
    } else if (!strcmp(bleBuff, "<")) {
        setpointAngle = SETPOINT_0;
        bleDirection = LEFT; // Turn Left
    } else if (!strcmp(bleBuff, ">")) {
        setpointAngle = SETPOINT_0;
        bleDirection = RIGHT; // Turn Right
    } else {
        setpointAngle = SETPOINT_0;
        bleDirection = IDLE; // IDLE
    }
    startTime = millis(); // Reset Start Time
    directionCount = 0; // Reset Direction Count
}

float normalizePWM(float u_t, float adjustedPWM) {
    float dutyCycle = (abs(u_t) - DEADZONE_PWM) / (VCC - DEADZONE_PWM) *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : ((dutyCycle < 0) ? 0 : dutyCycle); // Clamp Duty Cycle to [0, 1]
}

void moveForward(float dutyCycle) {
    moveSlowDecay(MotorA, CW, dutyCycle);
    moveSlowDecay(MotorB, CW, dutyCycle);
}

void moveReverse(float dutyCycle) {
    moveSlowDecay(MotorA, CCW, dutyCycle);
    moveSlowDecay(MotorB, CCW, dutyCycle);
}

void turnLeft(float u_t, float scaleFactor) {
    moveSlowDecay(MotorA, CW, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, CCW, normalizePWM(u_t, scaleFactor));
}

void turnRight(float u_t, float scaleFactor) {
    moveSlowDecay(MotorA, CCW, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, CW, normalizePWM(u_t, scaleFactor));
}

void drive(float u_t, float errorAngle) {
    currDutyCycle = normalizePWM(u_t, 0);

    if (millis() - startTime >= 1500) {
        startTime = currTime; // Reset Start Time
        setpointAngle = SETPOINT_0;
        bleDirection = IDLE; // Stop Robot
    }

    if (redAlert) {
        startTime = currTime; // Reset Start Time
        setpointAngle = SETPOINT_0 + 1.5 * ANGLE_TILT;
        bleDirection = REVERSE;
    }

    switch (bleDirection) {
        case FORWARD:
            if (forwardAlert) setpointAngle = SETPOINT_0 + 0.5 * ANGLE_TILT;

            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
        break;
        case REVERSE:
            if (reverseAlert) setpointAngle = SETPOINT_0 - ANGLE_TILT;

            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
        break;
        case LEFT:
            if ((directionCount++ % 3 == 0) || (abs(errorAngle) > MAX_ERR_ANGLE)) {
                if (u_t > 0) moveForward(currDutyCycle);
                else moveReverse(currDutyCycle);
            } else {
                turnLeft(currDutyCycle, 0.6);
            }
        break;
        case RIGHT:
            if ((directionCount++ % 3 == 0) || (abs(errorAngle) > MAX_ERR_ANGLE)) {
                if (u_t > 0) moveForward(currDutyCycle);
                else moveReverse(currDutyCycle);
            } else {
                turnRight(currDutyCycle, 0.5);
            }
        break;
        default:
            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
        break;
    }
    return;
}