#pragma once
#include <cstdint>
#include <sstream>
const int LOW = 0, HIGH = 1, INPUT = 0, OUTPUT = 1, INPUT_PULLUP = 2;
const uint8_t A0 = 14;
const uint8_t LED_BUILTIN = 13;
#define F(value) value
struct SerialStub {
  std::ostringstream output;
  void begin(int) {}
  template <typename T> void print(T value) { output << value; }
  template <typename T> void println(T value) { output << value << '\n'; }
  void println() { output << '\n'; }
};
extern SerialStub Serial;
uint32_t millis();
void pinMode(uint8_t, int);
int digitalRead(uint8_t);
int analogRead(uint8_t);
void digitalWrite(uint8_t, int);
void delayMicroseconds(unsigned int);
unsigned long pulseIn(uint8_t, uint8_t, unsigned long);
