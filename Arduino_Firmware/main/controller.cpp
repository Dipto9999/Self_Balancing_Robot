#include "controller.h"

const int VCC = 10.4; // 10.4 V

/* PID Controller Variables */
float SETPOINT_0; // Setpoint for PID Controller
float setpointAngle; // Reference Value, r_t (Angle = 180°)
float measuredAngle; // Output Value, y_t (Angle)

float errorAngle; // Error Value, e_t = r_t - y_t
float prevErrorAngle; // Previous Error Value, e_(t-1)

float Kp; // Proportional Gain
float Ki; // Integral Gain
float Kd; // Derivative Gain

float errorAccumulation; // Accumulated Error Value, ∑e_t
float errorDifference; // Derivative Error Value, e_t - e_(t-1) / dt
float u_t; // Control Signal

float currDutyCycle; // Current PWM Duty Cycle
int bleDirection; // Current Direction

void setupController() {
    Kp = 0.7; // Proportional Gain
    Ki = 11.25; // Integral Gain
    Kd = 0.055; // Derivative Gain

    SETPOINT_0 = -0.15;
    setpointAngle = SETPOINT_0; // Reference Value, r_t (Angle = 180°)
    errorAngle = 0.0; // Error Value, e_t = r_t - y_t
    prevErrorAngle = 0.0; // Previous Error Value, e_(t-1)

    u_t = 0.0; // Control Signal
    errorAccumulation = 0.0; // Accumulated Error Value, ∑e_t
    errorDifference = 0.0; // Derivative Error Value, e_t - e_(t-1) / dt

    bleDirection = IDLE; // Set Default Direction
    currDutyCycle = 0.5; // Set Default PWM Value

    pinMode(DISABLE_INTEGRAL_BUTTON, INPUT_PULLUP);
}

void setupMotors() {
    setupPWM(); // Initialize PWM Pins
    setupController(); // Initialize PID Controller
    Serial.println("Motors Initialized!");
}

void balanceRobot(int bleDirection) {
    // Get Measured Angle
    measuredAngle = Angles.Complementary; // Complementary Filter
    errorAngle = setpointAngle - measuredAngle; // e_t = r_t - y_t
    errorDifference = (errorAngle - prevErrorAngle) / dt; // e_t - e_(t-1) / dt
    errorAccumulation += (errorAngle * dt);

    // Reset Accumulated Error Value ∑e_t
    if (digitalRead(DISABLE_INTEGRAL_BUTTON) == LOW) errorAccumulation = 0;

    // Calculate Control Signal : u_t = Kp * e_t + Ki * ∑e_t + Kd * (e_t - e_(t-1) / dt)
    u_t = (Kp * errorAngle) + (Ki * errorAccumulation) + (Kd * errorDifference);
    drive(u_t, errorAngle);

    prevErrorAngle = errorAngle; // Update Previous Error Value
    return;
}

void updateParamBLE(const char* bleBuff) {
    char paramType[STD_BUFFSIZE] = {0};
    char valueStr[STD_BUFFSIZE] = {0};
    float newValue = 0.0f;

    char returnStr[STD_BUFFSIZE] = {0};

    char cmd[STD_BUFFSIZE] = {0};
    strcpy(cmd, bleBuff); // Copy Command to Local Buffer

    int start = 0;
    while ((cmd[start] == ' ') || (cmd[start] == '\t')) start++; // Skip Leading Whitespace

    if (
      strncmp(&cmd[start], "k=", 2) == 0 ||
      strncmp(&cmd[start], "set=", 4) == 0 ||
      strncmp(&cmd[start], "Kp=", 3) == 0 ||
      strncmp(&cmd[start], "Ki=", 3) == 0 ||
      strncmp(&cmd[start], "Kd=", 3) == 0
    ) {
      char* equalsPos = strchr(&cmd[start], '='); // Find '=' Position
      if (!equalsPos) return;

      int paramLength = equalsPos - &cmd[start]; // Length of Parameter Name
      strncpy(paramType, &cmd[start], paramLength); // Copy Parameter Name
      paramType[paramLength] = '\0'; // Null-Termination

      strcpy(valueStr, equalsPos + 1); // Copy Value String
      newValue = atof(valueStr); // Convert Value String to Float

      if (strcmp(paramType, "k") == 0) k = newValue;
      else if (strcmp(paramType, "set") == 0) SETPOINT_0 = newValue;
      else if (strcmp(paramType, "Kp") == 0) Kp = newValue;
      else if (strcmp(paramType, "Ki") == 0) Ki = newValue;
      else if (strcmp(paramType, "Kd") == 0) Kd = newValue;
      else return; // Invalid Command

      sprintf(returnStr, "NewVal: %s=%s", paramType, valueStr);
      customCharacteristic.writeValue(returnStr);
    }
    return; // Exit Function
}


void updateParamSerial() {
    int parseIndex;
    String paramType, valueStr;
    if (Serial.available() > 0) {
      // Check for PID Parameter Update Command
      String command = Serial.readStringUntil('\n');
      command.trim(); // Remove Leading/Trailing Whitespace

      if (
        command.startsWith("k=") ||
        command.startsWith("set=") ||
        command.startsWith("Kp=") ||
        command.startsWith("Ki=") ||
        command.startsWith("Kd=")
      ) {
        parseIndex = command.indexOf('=');
        paramType = command.substring(0, parseIndex);
        valueStr = command.substring(parseIndex + 1);

        float newValue = valueStr.toFloat();

        if (valueStr != "0" && newValue == 0) {
          Serial.println("Invalid Value!");
          return;
        }

        if (paramType == "k") k = newValue;
        else if (paramType == "set") SETPOINT_0 = newValue;
        else if (paramType == "Kp") Kp = newValue;
        else if (paramType == "Ki") Ki = newValue;
        else if (paramType == "Kd") Kd = newValue;

        Serial.print("\r\nk: ");
        Serial.print(k, 3);
        Serial.print(" Setpoint: ");
        Serial.println(setpointAngle, 3);

        Serial.print("Kp: ");
        Serial.print(Kp, 4);
        Serial.print(" Ki: ");
        Serial.print(Ki, 4);
        Serial.print(" Kd: ");
        Serial.print(Kd, 4);
      }
    }
  }
