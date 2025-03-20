#include "ble_movement.h"

#define ERROR_ANGLE_MAXIMUM 1
#define PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT 0.05

void changeDirection(const char* bleBuff) {
    if (!strcmp(bleBuff, "^") && !forwardAlert) bleDirection = FORWARD; // Drive
    else if (!strcmp(bleBuff, "v") && !reverseAlert) bleDirection = REVERSE; // Reverse
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

float calculateNewDutyCycle(float u_t, float pwmPercentage)
{
    float dutyCycle = abs(u_t) / VCC *  (1 + pwmPercentage); // Convert Control Signal to Duty Cycle
    return (dutyCycle > 1) ? 1 : dutyCycle; // Limit Duty Cycle to 100%
}


void bleMovement_Handle(float u_t, float errorAngle)
{
    float dutyCycleA = calculateNewDutyCycle(u_t, 0);
    switch (bleDirection)
    {
        case FORWARD:
            // if errorAngle > 0, then should move FORWARD to compensate
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM)
            {
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);    
            }  
        
        case REVERSE:
            if (errorAngle < 0 && errorAngle > -ERROR_ANGLE_MAXIMUM)
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT); 

        case LEFT: 
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM)
            {
                dutyCycleA = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
                float dutyCycleB = calculateNewDutyCycle(u_t, PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT); 
                turnLeft(dutyCycleA, dutyCycleB);
                return;    
            }

        
        case RIGHT:
            if (errorAngle > 0 && errorAngle < ERROR_ANGLE_MAXIMUM)
            {
                dutyCycleA = calculateNewDutyCycle(u_t, PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);
                float dutyCycleB = calculateNewDutyCycle(u_t, -PWM_PERCENTAGE_INCREASE_OR_DECREASE_FOR_MOVEMENT);  
                turnRight(dutyCycleA, dutyCycleB);
                return;
            }
            break;
        case IDLE:
            
            break;
        default:
            // Handle unexpected values, if necessary
            break;
        
        if (u_t > 0)
            moveForward(dutyCycleA);
        else
            moveReverse(dutyCycleA);
    }
}
