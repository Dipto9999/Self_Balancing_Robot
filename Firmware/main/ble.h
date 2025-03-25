#ifndef BLE_H
#define BLE_H

#include "Arduino_BMI270_BMM150.h"
#include <ArduinoBLE.h>
#include "serial.h"

#define BUFFER_SIZE 20

extern BLEService customService;
extern BLECharacteristic customCharacteristic;

extern char buffBLE[BUFFER_SIZE];

/* Function Prototypes */
extern void setupBLE();

extern void connectBLE(BLEDevice central);
extern void disconnectBLE(BLEDevice central);
extern void rxBLE(BLEDevice central, BLECharacteristic characteristic);

#endif