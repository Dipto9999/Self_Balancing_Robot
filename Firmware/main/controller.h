#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "pwm.h"
#include "angle.h"
#include "gpio.h"

enum DirRobot {REVERSE, FORWARD, LEFT, RIGHT, PARK};

struct ConfigPWM {
    float RPM_25;
    float RPM_50;
    float RPM_75;
    float RPM_100;
};

extern ConfigPWM ConfigMotor;

extern const int VCC;

/* PID Controller Variables */
extern float setpointAngle; // Reference Value, r_t (Angle = 180°)
extern float measuredAngle; // Output Value, y_t (Angle)

extern float errorAngle; // Error Value, e_t = r_t - y_t
extern float prevErrorAngle; // Previous Error Value, e_(t-1)

extern float Kp; // Proportional Gain
extern float Ki; // Integral Gain
extern float Kd; // Derivative Gain

extern float errorAccumulation; // Accumulated Error Value, ∑e_t
extern float errorDifference; // Derivative Error Value, e_t - e_(t-1) / dt
extern float u_t; // Control Signal

extern float currDutyCycle;
extern int bleDirection;

/* Function Prototypes */
void setupMotors();
void setupController();

void changeDirection(const char* bleBuff);
void balanceRobot(int direction);

void turnLeft();
void turnRight();

#endif