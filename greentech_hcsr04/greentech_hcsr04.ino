// Uno R3 + HC-SR04 + LDR + onboard LED. USB-powered desk prototype.
// HC-SR04 measures object distance; this is not a room occupancy detector.
#include <Arduino.h>

const uint8_t TRIG_PIN = 2;
const uint8_t ECHO_PIN = 7;
const uint8_t OVERRIDE_PIN = 3; // Connect to GND to force the LED ON.
const uint8_t LED_PIN = LED_BUILTIN;
const uint8_t LIGHT_PIN = A0;

// Change these values, then upload again.
const bool CSV_OUTPUT = false;       // Set true only for record_csv.py.
const uint16_t ENTER_MM = 500;       // Enter when closer than 50 cm.
const uint16_t EXIT_MM = 600;        // Remain near through 60 cm.
const uint32_t HOLD_MS = 10000UL;    // Later room trial: 600000UL.
const uint32_t SAMPLE_MS = 100UL;    // More than 60 ms between triggers.
const uint32_t LOG_MS = 1000UL;
const uint32_t ECHO_TIMEOUT_US = 25000UL;
const uint8_t FAILURES_TO_FAULT = 3;

uint32_t lastSample = 0;
uint32_t lastLog = 0;
uint32_t lastNear = 0;
uint16_t distanceMm = 0;
uint16_t lightRaw = 0;
uint8_t failedReadings = 0;
bool echoOk = false;
bool nearObject = false;
bool seenNear = false;
bool fault = false;
bool overrideOn = false;
bool lampOn = true;

void logRow(uint32_t now) {
  if (!CSV_OUTPUT) {
    Serial.print(F("Distance: "));
    if (echoOk) {
      Serial.print(distanceMm / 10);
      Serial.print('.');
      Serial.print(distanceMm % 10);
      Serial.print(F(" cm"));
    } else {
      Serial.print(F("NO VALID ECHO (check target and wiring)"));
    }
    Serial.print(F(" | Light (raw): "));
    Serial.print(lightRaw);
    Serial.print(F(" | LED: "));
    Serial.print(lampOn ? F("ON") : F("OFF"));
    if (overrideOn) Serial.print(F(" | Manual override"));
    if (fault) Serial.print(F(" | Sensor fault: LED forced ON"));
    Serial.println();
    return;
  }
  Serial.print(now);
  Serial.print(',');
  if (echoOk) {
    Serial.print(distanceMm / 10);
    Serial.print('.');
    Serial.print(distanceMm % 10);
  } // Empty distance means invalid, never zero centimetres.
  Serial.print(','); Serial.print(echoOk ? 1 : 0);
  Serial.print(','); Serial.print(lightRaw);
  Serial.print(','); Serial.print(echoOk && nearObject ? 1 : 0);
  Serial.print(','); Serial.print(fault ? 1 : 0);
  Serial.print(','); Serial.print(overrideOn ? 1 : 0);
  Serial.print(','); Serial.println(lampOn ? 1 : 0);
}

void setup() {
  digitalWrite(LED_PIN, HIGH); // Conservative output until first measurement.
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  pinMode(OVERRIDE_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  if (CSV_OUTPUT)
    Serial.println(F("uptime_ms,distance_cm,echo_ok,ldr_raw,near,fault,override,lamp"));
  else
    Serial.println(F("HC-SR04 ready. Distance in cm; light is raw 0-1023, not lux."));
}

void loop() {
  uint32_t now = millis();
  const bool previousLamp = lampOn;
  const bool previousFault = fault;

  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    const uint32_t duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
    now = millis(); // Include time spent waiting for the echo.
    echoOk = duration >= 116UL && duration <= 23200UL; // Nominal 2-400 cm.
    lightRaw = analogRead(LIGHT_PIN);

    if (echoOk) {
      distanceMm = (uint16_t)(duration * 10UL / 58UL);
      failedReadings = 0;
      fault = false; // One valid measurement clears this bench fault.
      if (nearObject) nearObject = distanceMm <= EXIT_MM;
      else nearObject = distanceMm < ENTER_MM;
      if (nearObject) {
        seenNear = true;
        lastNear = now;
      }
    } else {
      if (failedReadings < FAILURES_TO_FAULT) ++failedReadings;
      fault = failedReadings >= FAILURES_TO_FAULT;
      // An invalid reading never refreshes the vacancy timer.
    }
  }

  overrideOn = digitalRead(OVERRIDE_PIN) == LOW;
  const bool held = seenNear && now - lastNear < HOLD_MS;
  // During the first 100 ms there is no distance result yet.
  const bool awaitingFirstSample = lastSample == 0;
  lampOn = overrideOn || fault || awaitingFirstSample || held;
  digitalWrite(LED_PIN, lampOn ? HIGH : LOW);

  if (lampOn != previousLamp || fault != previousFault || now - lastLog >= LOG_MS) {
    lastLog = now;
    logRow(now);
  }
}
