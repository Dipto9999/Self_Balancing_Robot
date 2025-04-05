
#include "gpio.h"

bool stmConnected = false;
bool forwardAlert = false;
bool reverseAlert = false;
bool botEnabled = false;

void setupGPIO() {
    pinMode(PIN_STM32, INPUT_PULLUP);
    if (digitalRead(PIN_STM32) == LOW) {
        Serial.println("STM32 Not Connected!");
        stmConnected = false; // STM32 Connected
        return; // STM32 Not Connected
    } else {
        Serial.println("STM32 Connected!");
        stmConnected = true; // STM32 Connected

        pinMode(PIN_RFID_DISABLED, INPUT_PULLUP);
        pinMode(PIN_FORWARD_ALERT, INPUT_PULLUP);
        pinMode(PIN_REVERSE_ALERT, INPUT_PULLUP);
    }
}

void checkRFID() {
    if (digitalRead(PIN_RFID_DISABLED) == LOW) botEnabled = true;
    else botEnabled = false;
}

void checkForwardAlert() {
    if (digitalRead(PIN_FORWARD_ALERT) == LOW) forwardAlert = true;
    else forwardAlert = false;
}

void checkReverseAlert() {
    if (digitalRead(PIN_REVERSE_ALERT) == LOW) reverseAlert = true;
    else reverseAlert = false;
}