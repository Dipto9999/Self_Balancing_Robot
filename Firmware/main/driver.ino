#include "driver.h"

int balanceCounter = 0; // Counter for Balance Control Loop
float startTime, currTime; // Time Variables for Control Loop

void changeDirection(const char* bleBuff) {
    // if (!strcmp(bleBuff, "^") && !forwardAlert) bleDirection = FORWARD; // Drive
    // else if (!strcmp(bleBuff, "v") && !reverseAlert) bleDirection = REVERSE; // Reverse

    balanceCounter = 0; // Reset Balance Counter
    startTime = millis(); // Reset Start Time
    if (!strcmp(bleBuff, "^")) {
        bleDirection = FORWARD; // Drive
    } else if (!strcmp(bleBuff, "v")) {
        bleDirection = REVERSE; // Reverse
    }
    else if (!strcmp(bleBuff, "<")) {
        setpointAngle = SETPOINT_0;
        bleDirection = LEFT; // Turn Left
    } else if (!strcmp(bleBuff, ">")) {
        setpointAngle = SETPOINT_0;
        bleDirection = RIGHT; // Turn Right
    } else {
        setpointAngle = SETPOINT_0;
        bleDirection = IDLE; // IDLE
    }
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
    /*
    DirPWM direction = (u_t > 0) ? CW : CCW;
    moveSlowDecay(MotorA, direction, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, direction, normalizePWM(u_t, -scaleFactor));
    */

    moveSlowDecay(MotorA, CW, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, CCW, normalizePWM(u_t, 0));

}

void turnRight(float u_t, float scaleFactor) {
    DirPWM direction = (u_t > 0) ? CW : CCW;
    moveSlowDecay(MotorA, direction, normalizePWM(u_t, -scaleFactor));
    moveSlowDecay(MotorB, direction, normalizePWM(u_t, scaleFactor));

     /*
    moveSlowDecay(MotorA, CW, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, CCW, normalizePWM(u_t, 0));
    */
}

float normalizePWM(float u_t, float adjustedPWM) {
    float dutyCycle = (abs(u_t) - 0.055) / (VCC - 0.055) *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : ((dutyCycle < 0) ? 0 : dutyCycle); // Clamp Duty Cycle to [0, 1]
}

void drive(float u_t, float errorAngle) {
    float dutyCycle = normalizePWM(u_t, 0);

    if (millis() - startTime >= 2000) {
        startTime = currTime; // Reset Start Time
        setpointAngle = SETPOINT_0;
        bleDirection = IDLE; // Stop Robot
    }

    switch (bleDirection) {
        case FORWARD: // ToDo: Overshoots
            if (errorAngle >= 0 && errorAngle < 0.5) setpointAngle = SETPOINT_0;
            else if (errorAngle < 0 && errorAngle > -0.5) setpointAngle = SETPOINT_0 + ANGLE_TILT;

            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
            break;
        case REVERSE:
            if (errorAngle >= 0 && errorAngle < 0.5) setpointAngle = SETPOINT_0 - ANGLE_TILT;
            else if (errorAngle < 0 && errorAngle > -0.5) setpointAngle = SETPOINT_0;

            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
            break;
        case LEFT:
            if (errorAngle >= 0 && errorAngle < 0.5) {
                turnLeft(u_t, -0.03);
            }
            else if (errorAngle < 0 && errorAngle > -0.5) {
                turnLeft(u_t, 0.03);
            } else {
                if (u_t > 0) moveForward(dutyCycle);
                else moveReverse(dutyCycle);
            }
            break;
        case RIGHT: // ToDo: Does Not Work, Test
            if (errorAngle >= 0 && errorAngle < 0.5) {
                turnRight(u_t, -0.03);
            }
            else if (errorAngle < 0 && errorAngle > -0.5) {
                turnRight(u_t, 0.03);
            } else {
                if (u_t > 0) moveForward(dutyCycle);
                else moveReverse(dutyCycle);
            }
        break;
        default:
            if (u_t > 0) moveForward(dutyCycle);
            else moveReverse(dutyCycle);
        break;
    }
    currDutyCycle = dutyCycle; // Update Current Duty Cycle
}