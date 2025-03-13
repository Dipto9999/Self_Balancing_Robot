#include <Arduino.h>
#include <mbed.h>

// Create a Ticker object
mbed::Ticker TimerTicker;
#define LED_BUILTIN A0

// This is our ISR which toggles the built-in LED.
void timerISR() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
  // Initialize Serial for debugging.
  Serial.begin(115200);

  // Set the built-in LED as output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Attach the ticker to call timerISR every 1.0 second (i.e. 1 Hz).
  TimerTicker.attach(&timerISR, 0.1f);

  Serial.println("Ticker ISR started: LED toggles every 1 second.");
}

void loop() {
  delay(2000);
  // The main loop remains empty since the ticker ISR does the work.
}