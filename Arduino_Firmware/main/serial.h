#ifndef SERIAL_H
#define SERIAL_H

#include "Arduino_BMI270_BMM150.h"

extern String serialMsg;

/* Function Prototypes */
void setupSerial();
void handleData(char expectedByte, String txData);

#endif