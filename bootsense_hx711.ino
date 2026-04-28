// ======================================================
// Boot Sense — Final Firmware
// HX711 + Hysteresis Alert Logic + LED + Vibrator
//
// Board:       Arduino MKR WiFi 1010 (SAMD)
// Load cells:  2x strain-gauge (wired in parallel to one HX711 channel)
// Amplifier:   HX711 (DOUT → A5, SCK → A3) — power at 3.3V!
// Vibrator:    Coin motor via transistor → Pin 6
// LED:         Status indicator (220Ω to GND) → Pin 7
//
// University of Toledo — BIOE 4420 Senior Design
// ======================================================

#include "HX711.h"

// ─── HX711 Pins ───────────────────────────────────────
const int DOUT_PIN = A5;   // HX711 DOUT → A5
const int SCK_PIN  = A3;   // HX711 SCK  → A3

// ─── Output Pins ──────────────────────────────────────
const int VIB_PIN = 6;     // Coin vibrator (through transistor/driver)
const int LED_PIN = 7;     // Status LED (through 220Ω resistor to GND)

HX711 scale;

// ─── Calibration Values ───────────────────────────────
// HOW TO CALIBRATE:
//   1. Place the empty boot on the ground, read raw value from Serial Monitor.
//      Set EMPTY to that average.
//   2. Have the patient stand fully on the boot, read raw value.
//      Set STAND to that average.
//   3. Re-flash the board.
const long EMPTY = -685000L;   // Raw ADC — no weight on boot
const long STAND = -525000L;   // Raw ADC — full patient body weight

// Derived: number of ADC counts that equals 100% body weight
const long DELTA = STAND - EMPTY;

// ─── Clinical Thresholds ──────────────────────────────
// Partial weight-bearing zone (clinically: ~20–50% BW)
const float PCT_ON  = 0.30f;  // Alert activates at 30% BW
const float PCT_OFF = 0.20f;  // Alert deactivates at 20% BW (hysteresis)

// Minimum time above threshold before alert fires (avoids transient steps)
const unsigned long HOLD_MS = 300;

// ─── State Variables ──────────────────────────────────
bool vibOn = false;
unsigned long aboveStart = 0;

// ======================================================
// Averaging: reads n samples and returns the mean.
// HX711 outputs ~10 samples/sec at 10 Hz mode.
// ======================================================
long readAverage(int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) {
    while (!scale.is_ready()) { /* wait for conversion */ }
    sum += scale.read();
  }
  return sum / n;
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(9600);
  Serial.println("=== Boot Sense — HX711 Clinical Logic ===");
  Serial.println("Output: raw ADC value  |  %BW");

  scale.begin(DOUT_PIN, SCK_PIN);

  pinMode(VIB_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(VIB_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
}

// ======================================================
// MAIN LOOP
// ======================================================
void loop() {
  // 1) Read and average sensor samples
  long reading = readAverage(10);

  // 2) Convert raw ADC → percent body weight
  //    0.0 = no load, 1.0 = full body weight
  float pctBW = 0.0f;
  if (DELTA != 0) {
    pctBW = (float)(reading - EMPTY) / (float)DELTA;
  }

  // Clamp to a sane range (allow up to 150% BW)
  if (pctBW < 0.0f)  pctBW = 0.0f;
  if (pctBW > 1.5f)  pctBW = 1.5f;

  // Debug output — paste into Serial Plotter for visualization
  Serial.print("raw = ");
  Serial.print(reading);
  Serial.print("   %BW = ");
  Serial.println(pctBW * 100.0f, 1);

  // 3) Clinical hysteresis logic with hold-time requirement
  unsigned long now = millis();

  if (!vibOn) {
    // Alert is OFF → check if we should turn it ON
    if (pctBW >= PCT_ON) {
      if (aboveStart == 0) {
        aboveStart = now;                   // start hold timer
      } else if (now - aboveStart >= HOLD_MS) {
        vibOn = true;                       // sustained → alert ON
        Serial.println(">>> ALERT: weight limit exceeded!");
      }
    } else {
      aboveStart = 0;                       // dropped below threshold → reset
    }
  } else {
    // Alert is ON → turn OFF only when below lower threshold
    if (pctBW <= PCT_OFF) {
      vibOn = false;
      aboveStart = 0;
      Serial.println("--- Alert cleared.");
    }
  }

  // 4) Drive actuators — LED mirrors vibrator for visual debugging
  digitalWrite(VIB_PIN, vibOn ? HIGH : LOW);
  digitalWrite(LED_PIN, vibOn ? HIGH : LOW);

  delay(50);   // ~20 Hz effective sample rate after averaging
}
