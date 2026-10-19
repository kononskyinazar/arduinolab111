#include <cassert>
#include <iostream>
#include "test-support/Arduino.h"
SerialStub Serial;
uint32_t clockMs = 0;
int pirLevel = LOW, lightValue = 512, led = LOW;
uint32_t millis() { return clockMs; }
void pinMode(uint8_t, int) {}
int digitalRead(uint8_t) { return pirLevel; }
int analogRead(uint8_t) { return lightValue; }
void digitalWrite(uint8_t pin, int value) { if (pin == LED_BUILTIN) led = value; }
#include "sensor_check/sensor_check.ino"
void tick(uint32_t time) { clockMs = time; loop(); }
int main() {
  setup(); pirLevel = HIGH; tick(500); assert(led == LOW);
  tick(60000); assert(led == HIGH);
  pirLevel = LOW; tick(60001); assert(led == LOW); // No 10-second software hold.
  Serial.output.str(""); lightValue = 0; tick(60500);
  assert(Serial.output.str() == "READY | PIR OUT: LOW | Light A0: 0 | LED L: OFF\n");
  Serial.output.str(""); lightValue = 1023; pirLevel = HIGH; tick(61000);
  assert(Serial.output.str() == "READY | PIR OUT: HIGH | Light A0: 1023 | LED L: ON\n");
  pirLevel = LOW; tick(UINT32_MAX - 1); tick(500); assert(!warmingUp && led == LOW);
  std::cout << "PASS: diagnostic warmup, direct LED response, raw light display, rollover\n";
}
