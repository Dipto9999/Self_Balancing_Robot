
#include "buttons.h"

void setupButtons() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

bool handleButtons() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(20); // Debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            while (digitalRead(BUTTON_PIN) == LOW); // Wait for Button Release
            return true;
        }
    }
    return false;
}