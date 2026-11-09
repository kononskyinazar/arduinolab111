#include <cassert>
#include <iostream>
#include "test-support/Arduino.h"
SerialStub Serial;
uint32_t clockMs = 0;
int lightValue = 0, pirValue = LOW, externalLed = LOW, onboardLed = LOW;
uint32_t millis() { return clockMs; }
void pinMode(uint8_t, int) {}
int analogRead(uint8_t pin) { assert(pin == A0); return lightValue; }
int digitalRead(uint8_t pin) { assert(pin == 6); return pirValue; }
void digitalWrite(uint8_t pin, int value) {
  if (pin == 9) externalLed = value;
  if (pin == LED_BUILTIN) onboardLed = value;
}
#include "greentech_pir/greentech_pir.ino"
int main() {
  setup();
  assert(externalLed == LOW && onboardLed == LOW);
  for (int value : {0, 350, 599, 600, 650, 700, 701, 1023}) {
    for (int pir : {LOW, HIGH}) {
      lightValue = value; pirValue = pir; clockMs += 500; loop();
      const int expected = value >= 600 && value <= 700 ? HIGH : LOW;
      assert(externalLed == expected && onboardLed == expected);
    }
  }
  Serial.output.str("");
  lightValue = 650; pirValue = LOW; clockMs += 500; loop();
  assert(Serial.output.str() == "Light (raw): 650 | LED: ON | PIR OUT: LOW\n");
  std::cout << "PASS: 600-700 inclusive, outside range OFF, PIR independent, readable output\n";
}
