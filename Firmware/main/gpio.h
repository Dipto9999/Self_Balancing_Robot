#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino_BMI270_BMM150.h"

#define PIN_RFID_DISABLED A2
#define PIN_FORWARD_ALERT A0
#define PIN_REVERSE_ALERT A1

extern bool forwardAlert;
extern bool reverseAlert;

extern void setupGPIO();

extern void checkRFIDDisabled();
extern void checkForwardAlert();
extern void checkReverseAlert();

#endif