#include <veml3328.h>

void setup() {
    Serial.begin(115200);

    if (Veml3328.begin()) {
        Serial.println("Error: could not start VEML3328 library");
    }
}

void loop() {

    Serial.println();
    Serial.printf("Red: %u\r\n", Veml3328.getRed());
    Serial.printf("Green: %u\r\n", Veml3328.getGreen());
    Serial.printf("Blue: %u\r\n", Veml3328.getBlue());
    Serial.printf("IR: %u\r\n", Veml3328.getIR());
    Serial.printf("Clear: %u\r\n\r\n", Veml3328.getClear());

    delay(2000);
}