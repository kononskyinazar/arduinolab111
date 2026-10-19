#include <cassert>
#include <iostream>
#include "test-support/Arduino.h"
SerialStub Serial;
uint32_t clockMs = 0;
int pir = LOW, manualInput = HIGH, led = LOW;
uint32_t millis() { return clockMs; }
void pinMode(uint8_t, int) {}
int digitalRead(uint8_t pin) { return pin == 2 ? pir : manualInput; }
int analogRead(uint8_t) { return 512; }
void digitalWrite(uint8_t pin, int value) { if (pin == LED_BUILTIN) led = value; }
#include "greentech_pir/greentech_pir.ino"
void tick(uint32_t now) { clockMs = now; loop(); }
int main() {
  setup(); tick(50); assert(led == HIGH && warmingUp);
  tick(60000); assert(led == LOW && !warmingUp);
  pir = HIGH; tick(60050); assert(led == HIGH);
  Serial.output.str("");
  tick(61050);
  assert(Serial.output.str() == "Motion: YES | Light (raw): 512 | LED: ON\n");
  pir = LOW; tick(71000); assert(led == HIGH);
  tick(71050); assert(led == LOW);
  manualInput = LOW; tick(71100); assert(led == HIGH);
  manualInput = HIGH; tick(71150); assert(led == LOW);
  pir = HIGH; tick(UINT32_MAX - 1000);
  pir = LOW; tick(500); assert(led == HIGH && !warmingUp);
  tick(10000); assert(led == LOW && !warmingUp);
  std::cout << "PASS: PIR warmup, motion, hold, override, display, rollover\n";
}
