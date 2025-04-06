#ifndef GPIO_H
#define GPIO_H

#include "Arduino_BMI270_BMM150.h"

#define PIN_STM32 A0
#define PIN_RFID_DISABLED A2
#define PIN_FORWARD_ALERT A3
#define PIN_REVERSE_ALERT A4

extern bool stmConnected;
extern bool botEnabled;
extern bool forwardAlert;
extern bool reverseAlert;

extern void setupGPIO();

extern void checkRFID();
extern void checkForwardAlert();
extern void checkReverseAlert();

#endif