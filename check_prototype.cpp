#include <cassert>
#include <cstdint>
#include <iostream>
#include "test-support/Arduino.h"
SerialStub Serial;
uint32_t clockMs = 0;
unsigned long echoUs = 5800;
int manualInput = HIGH;
int ledOutput = LOW;
uint32_t millis() { return clockMs; }
void pinMode(uint8_t, int) {}
int digitalRead(uint8_t) { return manualInput; }
int analogRead(uint8_t) { return 500; }
void digitalWrite(uint8_t pin, int value) { if (pin == LED_BUILTIN) ledOutput = value; }
void delayMicroseconds(unsigned int) {}
unsigned long pulseIn(uint8_t, uint8_t, unsigned long) { return echoUs; }
#include "greentech_hcsr04/greentech_hcsr04.ino"
void sample(unsigned long duration) { echoUs = duration; clockMs += 100; loop(); }
int main() {
  setup(); loop(); assert(ledOutput == HIGH);
  sample(5800); assert(echoOk && !nearObject && ledOutput == LOW);
  sample(2900); assert(!nearObject); // Exactly 50 cm does not enter.
  sample(2842); assert(nearObject && ledOutput == HIGH); // 49 cm.
  sample(3190); assert(nearObject); // 55 cm maintains existing detection.
  sample(3480); assert(nearObject); // Exactly 60 cm remains near.
  const uint32_t lastDetection = clockMs;
  sample(3538); assert(!nearObject && ledOutput == HIGH); // 61 cm, hold active.
  clockMs = lastDetection + 9999; loop(); assert(ledOutput == HIGH);
  clockMs = lastDetection + 10099; loop(); assert(ledOutput == LOW);
  sample(3190); assert(!nearObject && ledOutput == LOW); // 55 cm cannot re-enter.
  sample(0); assert(!fault);
  sample(0); assert(!fault);
  sample(0); assert(fault && ledOutput == HIGH);
  sample(5800); assert(!fault && ledOutput == LOW);
  manualInput = LOW; loop(); assert(ledOutput == HIGH);
  manualInput = HIGH; loop(); assert(ledOutput == LOW);
  sample(115); sample(23201); sample(0); assert(fault);
  sample(116); assert(echoOk && !fault && nearObject);
  sample(23200); assert(echoOk && !nearObject);
  clockMs = UINT32_MAX - 1000; sample(2842);
  echoUs = 5800; clockMs = 500; loop(); assert(ledOutput == HIGH);
  clockMs = 10000; loop(); assert(ledOutput == LOW);
  // Verify the actual user-visible distance and invalid-echo messages.
  Serial.output.str("");
  echoOk = true; distanceMm = 324; lightRaw = 512;
  lampOn = true; fault = false; overrideOn = false;
  logRow(clockMs);
  assert(Serial.output.str() == "Distance: 32.4 cm | Light (raw): 512 | LED: ON\n");
  Serial.output.str("");
  echoOk = false; fault = true;
  logRow(clockMs);
  assert(Serial.output.str().find("NO VALID ECHO") != std::string::npos);
  assert(Serial.output.str().find("Sensor fault: LED forced ON") != std::string::npos);
  assert(Serial.output.str().find("32.4") == std::string::npos);
  std::cout << "PASS: control logic, distance display, invalid echo, fault display\n";
}
