// Uno R3: the LED is ON only for raw light readings from 600 through 700.
#include <Arduino.h>

const uint8_t LIGHT_PIN = A0;
const uint8_t PIR_PIN = 6; // Reported only; does not control the LED.
const uint8_t LED_PIN = 9;
const int LIGHT_MIN = 600;
const int LIGHT_MAX = 700;
uint32_t lastSample = 0;
uint32_t lastPrint = 0;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  Serial.println(F("LED ON for Light (raw) 600-700 inclusive. PIR does not control LED."));
}

void loop() {
  const uint32_t now = millis();
  if (now - lastSample < 50UL) return;
  lastSample = now;

  const int light = analogRead(LIGHT_PIN);
  const bool pirHigh = digitalRead(PIR_PIN) == HIGH;
  const bool ledOn = light >= LIGHT_MIN && light <= LIGHT_MAX;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);

  if (now - lastPrint >= 500UL) {
    lastPrint = now;
    Serial.print(F("Light (raw): "));
    Serial.print(light);
    Serial.print(F(" | LED: "));
    Serial.print(ledOn ? F("ON") : F("OFF"));
    Serial.print(F(" | PIR OUT: "));
    Serial.println(pirHigh ? F("HIGH") : F("LOW"));
  }
}
