#include <Arduino.h>
#include <mbed.h>
#include <chrono>

using namespace std::chrono_literals;

#include "controller.h"

static mbed::Ticker controlTicker;

static constexpr auto CONTROL_PERIOD = 20ms;

void setupISR() {
  controlTicker.attach(&timerISR, CONTROL_PERIOD);
}

void timerISR() {
  // getAngles(Angles);
  balanceRobot(bleDirection);
  // digitalWrite(PIN_RFID_DISABLED, !digitalRead(PIN_RFID_DISABLED));
}
