// Nano 33 BLE Sense Rev2 — Serial1 Hello/Hey Responder
// ---------------------------------------------------
// • Wire D1 (TX1) → D0 (RX1) and GND↔GND.
// • Pi will send "Hello {n}\n"; Arduino replies "Hey {n}\n".
// • Open the IDE Serial Monitor at 115200 baud to watch USB logs.

#include <Arduino.h>

int n;
void setup() {
    Serial.begin(115200);
    while (!Serial) ; // Wait for Native USB Serial Connection
    Serial1.begin(115200); // UART on D0/D1 (RX1/TX1)
    Serial.println("=== Raspberry-Pi/Arduino Serial Test ===");

    n = 0; // Initialize n
}

void RaspberryPiTest() {
    if (Serial1.available()) {
        String incoming = Serial1.readStringUntil('\n');

        incoming.trim(); // Strip "\r\n"
        Serial.print("RX: '");
        Serial.println(incoming);

        // Expect "Hello {n}"

        n += 1; // Increment n
        n = n % 100; // 0 <= n <= 99

        String outgoing = "Hey " + String(n);
        Serial1.println(outgoing);
        Serial.print("TX: '");
        Serial.println(outgoing);
    }
}

void loop() {
  RaspberryPiTest(); // Call the test function
}
