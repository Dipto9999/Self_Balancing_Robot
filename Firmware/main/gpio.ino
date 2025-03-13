
#include "gpio.h"

bool forwardAlert = false;
bool reverseAlert = false;

void setupGPIO() {
    // pinMode(PIN_FORWARD_ALERT, OUTPUT);
    pinMode(PIN_REVERSE_ALERT, INPUT_PULLUP);
}

void checkForwardAlert() {
    if (digitalRead(PIN_FORWARD_ALERT) == LOW) forwardAlert = true;
    else forwardAlert = false;
}

void checkReverseAlert() {
    if (digitalRead(PIN_REVERSE_ALERT) == LOW) reverseAlert = true;
    else reverseAlert = false;
}