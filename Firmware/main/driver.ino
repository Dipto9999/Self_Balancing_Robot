#include "driver.h"

int balanceCounter = 0; // Counter for Balance Control Loop
float startTime, currTime; // Time Variables for Control Loop

void changeDirection(const char* bleBuff) {
    balanceCounter = 0; // Reset Balance Counter
    startTime = millis(); // Reset Start Time

    // if (!strcmp(bleBuff, "^")) {
        //     bleDirection = FORWARD; // Drive
        // } else if (!strcmp(bleBuff, "v")) {
            //     bleDirection = REVERSE; // Reverse
    if (!strcmp(bleBuff, "^")) {
        setpointAngle = SETPOINT_0 + ANGLE_TILT;
        bleDirection = (!forwardAlert) ? FORWARD : IDLE; // Drive
    } else if (!strcmp(bleBuff, "v")) {
        setpointAngle = SETPOINT_0 - ANGLE_TILT;
        bleDirection = (!reverseAlert) ? REVERSE : IDLE; // Reverse
    } else if (!strcmp(bleBuff, "<")) {
        setpointAngle = SETPOINT_0;
        bleDirection = LEFT; // Turn Left
    } else if (!strcmp(bleBuff, ">")) {
        setpointAngle = SETPOINT_0;
        bleDirection = RIGHT; // Turn Right
    } else if (!strcmp(bleBuff, "X")) {
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
    moveSlowDecay(MotorA, CCW, normalizePWM(u_t, scaleFactor));
    moveSlowDecay(MotorB, CW, normalizePWM(u_t, 0));
}

void turnRight(float u_t, float scaleFactor) {
    moveSlowDecay(MotorA, CW, normalizePWM(u_t, 0));
    moveSlowDecay(MotorB, CCW, normalizePWM(u_t, scaleFactor));
}

float normalizePWM(float u_t, float adjustedPWM) {
    float dutyCycle = (abs(u_t) - 0.055) / (VCC - 0.055) *  (1 + adjustedPWM); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : ((dutyCycle < 0) ? 0 : dutyCycle); // Clamp Duty Cycle to [0, 1]
}

void drive(float u_t, float errorAngle) {
    currDutyCycle = normalizePWM(u_t, 0);

    // ToDo: Overshoots
    switch (bleDirection) {
        case FORWARD:
           if (abs(errorAngle) < 0.8 && balanceCounter++ % 2 == 0) {
                balanceCounter = 0;
                setpointAngle = SETPOINT_0 - ANGLE_TILT;
            } else {
                setpointAngle = SETPOINT_0;
            }

            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
            /*
            if (millis() - startTime >= 2000) {
                startTime = currTime; // Reset Start Time
                setpointAngle = SETPOINT_0;
                bleDirection = IDLE; // Stop Robot
            }
                */
            break;
        case REVERSE:
            if (abs(errorAngle) < 0.8 && balanceCounter++ % 2 == 0) {
                balanceCounter = 0;
                setpointAngle = SETPOINT_0 + ANGLE_TILT;
            } else {
                setpointAngle = SETPOINT_0;
            }

            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
            /*
            if (millis() - startTime >= 2000) {
                startTime = currTime; // Reset Start Time
                setpointAngle = SETPOINT_0;
                bleDirection = IDLE; // Stop Robot
            }
                */
            break;
        case LEFT:
            if (errorAngle < 0.8 && errorAngle > 0)
            {
                turnLeft(u_t, -0.05);
            }
            else if (errorAngle > -0.8 && errorAngle < 0) {
                turnLeft(u_t, 0.05);
            } else {
                if (u_t > 0) moveForward(currDutyCycle);
                else moveReverse(currDutyCycle);
            }

        break;

        case RIGHT:
            if (errorAngle < 0.8 && errorAngle > 0)
            {
                turnRight(u_t, -0.05);
            }
            else if (errorAngle > -0.8 && errorAngle < 0) {
                turnRight(u_t, 0.05);
            } else {
                if (u_t > 0) moveForward(currDutyCycle);
                else moveReverse(currDutyCycle);
            }

        
        break;
        default:
            if (u_t > 0) moveForward(currDutyCycle);
            else moveReverse(currDutyCycle);
            return;
        break;
        return;
    }
}