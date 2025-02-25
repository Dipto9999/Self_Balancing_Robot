#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "pwm.h"
#include "angle.h"

enum DirRobot {DRIVE, BACK, LEFT, RIGHT};

struct ConfigPWM {
    float RPM_25;
    float RPM_50;
    float RPM_75;
    float RPM_100;
};

extern ConfigPWM ConfigMotor;

extern XIN MotorA;
extern XIN MotorB;

/* PID Controller Variables */
extern float Kp; // Proportional Gain
extern float Ki; // Integral Gain
extern float Kd; // Derivative Gain

extern float setpointAngle; // Reference Value, r_t (Angle = 180°)
extern float measuredAngle; // Output Value, y_t (Angle)

extern float currErrorAngle; // Error Value, e_t = r_t - y_t
extern float prevErrorAngle; // Previous Error Value, e_(t-1)

extern float integralErrorAngle; // Integral Value, ∑(e_t * dt)
extern float derivativeErrorAngle; // Derivative Value, (e_t - e_(t-1)) / dt

extern float u_t; // Control Signal

extern float currDutyCycle;
extern int currDirection;

/* Function Prototypes */
void setupMotors();
void setupController();

void changeDirection(const char* bleBuff);
void balanceRobot(int direction);

#endif