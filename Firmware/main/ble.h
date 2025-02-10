#ifndef BLE_H
#define BLE_H

#include "Arduino_BMI270_BMM150.h"
#include <ArduinoBLE.h>

#define BUFFER_SIZE 20

extern void setupBLE();
extern void handleBLE();

#endif