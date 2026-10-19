// Uno R3 + HC-SR501 + LDR + onboard LED. Motion is not a people count.
#include <Arduino.h>

const uint8_t PIR_PIN = 2;
const uint8_t OVERRIDE_PIN = 3; // Optional jumper to GND forces LED ON.
const uint8_t LIGHT_PIN = A0;
const uint32_t HOLD_MS = 10000UL;
const uint32_t WARMUP_MS = 60000UL;
const uint32_t SAMPLE_MS = 50UL;
const uint32_t LOG_MS = 1000UL;

uint32_t lastSample = 0;
uint32_t lastLog = 0;
uint32_t lastMotion = 0;
bool warmingUp = true;
bool seenMotion = false;
bool motion = false;
bool lampOn = true;
bool overrideOn = false;
int lightRaw = 0;

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(OVERRIDE_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println(F("HC-SR501: wait 60 seconds for warmup. Light is raw 0-1023, not lux."));
}

void loop() {
  const uint32_t now = millis();
  const bool previousLamp = lampOn;
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;
    motion = digitalRead(PIR_PIN) == HIGH;
    lightRaw = analogRead(LIGHT_PIN);
    if (motion) {
      seenMotion = true;
      lastMotion = now;
    }
    if (warmingUp && now >= WARMUP_MS) warmingUp = false;
  }
  overrideOn = digitalRead(OVERRIDE_PIN) == LOW;
  lampOn = warmingUp || overrideOn || (seenMotion && now - lastMotion < HOLD_MS);
  digitalWrite(LED_BUILTIN, lampOn ? HIGH : LOW);

  if (lampOn != previousLamp || now - lastLog >= LOG_MS) {
    lastLog = now;
    Serial.print(F("Motion: "));
    Serial.print(warmingUp ? F("WARMING UP") : (motion ? F("YES") : F("NO")));
    Serial.print(F(" | Light (raw): "));
    Serial.print(lightRaw);
    Serial.print(F(" | LED: "));
    Serial.print(lampOn ? F("ON") : F("OFF"));
    if (overrideOn) Serial.print(F(" | Manual override"));
    Serial.println();
  }
}
