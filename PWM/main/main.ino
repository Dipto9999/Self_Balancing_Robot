#include "angle.h"
#include "pwm.h"

void setup() {
    setupSerial();
    setupIMU();
    setupPWM();
}

float angles[3] = {0, 0, 0};

void loop() {
    getAngles(angles);

    // Serial.print("Accel: ");
    // Serial.println(angles[0]);

    // Serial.print("Gyro: ");
    // Serial.println(angles[1]);

    // Serial.print("Complementary: ");
    // Serial.println(angles[2]);
}