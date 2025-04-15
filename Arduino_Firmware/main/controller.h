#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "serial.h"
#include "ble.h"
#include "pwm.h"
#include "angle.h"
#include "gpio.h"

#include "driver.h"

#define DISABLE_INTEGRAL_BUTTON 10

enum DirRobot {REVERSE, FORWARD, LEFT, RIGHT, IDLE};

extern const int VCC;

/* PID Controller Variables */
extern float SETPOINT_0; // Setpoint for PID Controller
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

extern void setupController();
extern void setupMotors();

extern void balanceRobot(int bleDirection);

extern void updateParamBLE(const char* bleBuff);
extern void updateParamSerial();

#endif