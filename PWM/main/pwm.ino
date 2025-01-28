#include "pwm.h"

// PWM 1 : Pins 3, 5
int PinAIN1 = 3;
int PinAIN2 = 5;

// PWM 2 :  Pins 6, 9
int PinBIN1 = 6;
int PinBIN2 = 9;

int val = 0;

void setupPWM() {
  pinMode(PinAIN1, OUTPUT);
  pinMode(PinAIN2, OUTPUT);
  pinMode(PinBIN1, OUTPUT);
  pinMode(PinBIN2, OUTPUT);
}

void writePWM(int pin) {
  // analogRead values go from 0 to 1023
  analogWrite(pin, val / 4); // analogWrite values from 0 to 255
}
