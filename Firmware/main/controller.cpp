#include <Arduino.h>
#include <mbed.h>
#include <chrono>

using namespace std::chrono_literals;

#include "controller.h"

static mbed::Ticker timerTicker;

static constexpr auto CONTROL_PERIOD = 20ms;

void setupISR() {
  timerTicker.attach(&timerISR, CONTROL_PERIOD);
}

void timerISR() {
  balanceRobot(bleDirection);
  digitalWrite(PIN_RFID_DISABLED, !digitalRead(PIN_RFID_DISABLED));
}
