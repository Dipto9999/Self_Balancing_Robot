#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino_BMI270_BMM150.h"

#define BUTTON_PIN A0

extern void setupButtons();

extern bool handleButtons();

#endif