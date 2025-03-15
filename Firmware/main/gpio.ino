
#include "gpio.h"

bool forwardAlert = false;
bool reverseAlert = false;

void setupGPIO() {
    pinMode(PIN_RFID_DISABLED, OUTPUT);
    pinMode(PIN_FORWARD_ALERT, INPUT_PULLUP);
    pinMode(PIN_REVERSE_ALERT, INPUT_PULLUP);

    digitalWrite(PIN_RFID_DISABLED, HIGH);
}

void checkRFIDDisabled() {
    while (digitalRead(PIN_RFID_DISABLED) == LOW);
}

void checkForwardAlert() {
    if (digitalRead(PIN_FORWARD_ALERT) == LOW) forwardAlert = true;
    else forwardAlert = false;
}

void checkReverseAlert() {
    if (digitalRead(PIN_REVERSE_ALERT) == LOW) reverseAlert = true;
    else reverseAlert = false;
}
