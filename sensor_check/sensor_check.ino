// Diagnostic only: read the actual PIR output and LDR input without lamp timers.
#include <Arduino.h>

const uint8_t PIR_PIN = 2;
const uint8_t LIGHT_PIN = A0;
const uint32_t WARMUP_MS = 60000UL;
uint32_t lastPrint = 0;
bool warmingUp = true;

void setup() {
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("SENSOR CHECK: keep PIR still; wait 60 seconds. LED L then follows PIR OUT directly."));
}

void loop() {
  const uint32_t now = millis();
  if (warmingUp && now >= WARMUP_MS) warmingUp = false;
  const bool pirHigh = digitalRead(PIR_PIN) == HIGH;
  const int light = analogRead(LIGHT_PIN);
  const bool ledOn = !warmingUp && pirHigh;
  digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);

  if (now - lastPrint >= 500UL) {
    lastPrint = now;
    Serial.print(warmingUp ? F("WARMUP") : F("READY"));
    Serial.print(F(" | PIR OUT: "));
    Serial.print(pirHigh ? F("HIGH") : F("LOW"));
    Serial.print(F(" | Light A0: "));
    Serial.print(light);
    Serial.print(F(" | LED L: "));
    Serial.println(ledOn ? F("ON") : F("OFF"));
  }
}
