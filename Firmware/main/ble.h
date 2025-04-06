#ifndef BLE_H
#define BLE_H

#include "Arduino_BMI270_BMM150.h"
#include <ArduinoBLE.h>
#include "serial.h"

#define STD_BUFFSIZE 20
#define CODE "0095"

extern BLEService customService;
extern BLECharacteristic customCharacteristic;

extern char buffBLE[STD_BUFFSIZE];
extern bool isAuthenticated;
extern bool pairPrompted;

/* Function Prototypes */
extern void setupBLE();

extern void connectBLE(BLEDevice central);
extern void disconnectBLE(BLEDevice central);

extern void authenticateBLE();
extern void rxBLE(BLEDevice central, BLECharacteristic characteristic);

#endif