#include <veml3328.h>

void setup() {
    Serial1.begin(115200);

    if (Veml3328.begin()) {
        Serial1.println("Error: could not start VEML3328 library");
    }
}

void loop() {

    Serial1.println();
    Serial1.printf("Red: %u\r\n", Veml3328.getRed());
    Serial1.printf("Green: %u\r\n", Veml3328.getGreen());
    Serial1.printf("Blue: %u\r\n", Veml3328.getBlue());
    Serial1.printf("IR: %u\r\n", Veml3328.getIR());
    Serial1.printf("Clear: %u\r\n\r\n", Veml3328.getClear());

    delay(2000);
}