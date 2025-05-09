#ifndef BLE_H
#define BLE_H

#include "Arduino_BMI270_BMM150.h"
#include <ArduinoBLE.h>

#include "serial.h"
#include "controller.h"

#define STD_BUFFSIZE 20
#define CODE "EVE"

extern BLEService customService;
extern BLECharacteristic customCharacteristic;

extern char buffBLE[STD_BUFFSIZE];
extern bool isAuthenticated, isPromptedBLE;

extern unsigned long lastBLETime;
extern const unsigned long BLE_INTERVAL;

/* Function Prototypes */
extern void setupBLE();

extern void connectBLE(BLEDevice central);
extern void disconnectBLE(BLEDevice central);

extern void authBLE();
extern void rxBLE(BLEDevice central, BLECharacteristic characteristic);

extern void checkEventBLE();

#endif