// ======================================================
// Boot Sense — Analog ADC Test Version (No HX711)
// Board: MKR WiFi 1010 (SAMD)
//
// Used for early-stage testing before integrating HX711.
// Load cells read directly via analog pins (less accurate —
// no instrumentation amplifier, susceptible to noise).
//
// Load cells → A0, A1
// Coin vibrator → Digital Pin 6
// ======================================================

// ─── Pins ─────────────────────────────────────────────
const int LOADCELL_1_PIN = A0;
const int LOADCELL_2_PIN = A1;
const int VIBRATOR_PIN   = 6;

// ─── Patient / Clinical Settings ──────────────────────
const float PATIENT_BODY_WEIGHT_LBS = 130.0;   // ← set patient body weight
const float ALLOWED_PWB_PERCENT     = 80.0;    // ← allowed % BW (lower for rehab)

// Margins around the cap
const float WARNING_MARGIN_PERCENT  = 0.10;    // 10% below cap
const float CRITICAL_MARGIN_PERCENT = 0.10;    // 10% above cap

// Computed in setup()
float WARNING_WEIGHT_LBS     = 0.0;
float MAX_ALLOWED_WEIGHT_LBS = 0.0;
float CRITICAL_WEIGHT_LBS    = 0.0;

// ─── Calibration ──────────────────────────────────────
float WEIGHT_CALIBRATION_FACTOR  = 0.02;       // ADC counts → lbs (tune with scale)
float SENSOR_WEIGHT_FACTORS[2]   = {1.0, 1.0}; // per-sensor gain trim

// ─── Sensor State ─────────────────────────────────────
int   sensorMin[2]          = {4095, 4095};
int   sensorMax[2]          = {0, 0};
int   rawReadings[2]        = {0, 0};
int   calibratedReadings[2] = {0, 0};
float individualWeights[2]  = {0.0, 0.0};
float estimatedWeight       = 0.0;

// ─── Timing & Alert State ─────────────────────────────
unsigned long lastWeightAlert              = 0;
const unsigned long WEIGHT_ALERT_INTERVAL  = 3000;
bool weightWarningActive = false;
bool criticalWeightAlert = false;

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  analogReadResolution(12);   // 12-bit ADC on SAMD (0–4095)

  pinMode(VIBRATOR_PIN, OUTPUT);
  digitalWrite(VIBRATOR_PIN, LOW);

  MAX_ALLOWED_WEIGHT_LBS = PATIENT_BODY_WEIGHT_LBS * (ALLOWED_PWB_PERCENT / 100.0);
  WARNING_WEIGHT_LBS     = MAX_ALLOWED_WEIGHT_LBS * (1.0 - WARNING_MARGIN_PERCENT);
  CRITICAL_WEIGHT_LBS    = MAX_ALLOWED_WEIGHT_LBS * (1.0 + CRITICAL_MARGIN_PERCENT);

  Serial.println("=== Boot Sense — Analog Test Version ===");
  Serial.print("Patient BW: ");      Serial.print(PATIENT_BODY_WEIGHT_LBS); Serial.println(" lbs");
  Serial.print("Allowed PWB: ");     Serial.print(ALLOWED_PWB_PERCENT);     Serial.println("% BW");
  Serial.print("Warning  > ");       Serial.print(WARNING_WEIGHT_LBS);      Serial.println(" lbs");
  Serial.print("Cap (max) > ");      Serial.print(MAX_ALLOWED_WEIGHT_LBS);  Serial.println(" lbs");
  Serial.print("Critical > ");       Serial.print(CRITICAL_WEIGHT_LBS);     Serial.println(" lbs");

  Serial.println("Calibrating sensors (5s) — keep boot unloaded...");
  calibrateSensors();
  Serial.println("Calibration done. Apply load to test.");
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  readLoadCells();
  calculateWeight();
  checkWeightLimits();
  displayReadings();
  delay(100);   // ~10 Hz
}

// ======================================================
// CALIBRATION — find min/max range per sensor
// ======================================================
void calibrateSensors() {
  unsigned long t0 = millis();
  while (millis() - t0 < 5000) {
    for (int i = 0; i < 2; i++) {
      int r = analogRead(getSensorPin(i));
      if (r < sensorMin[i]) sensorMin[i] = r;
      if (r > sensorMax[i]) sensorMax[i] = r;
    }
    delay(10);
  }
  for (int i = 0; i < 2; i++) {
    Serial.print("L"); Serial.print(i + 1);
    Serial.print(": "); Serial.print(sensorMin[i]);
    Serial.print(" – "); Serial.println(sensorMax[i]);
  }
}

// ======================================================
// SENSOR READING
// ======================================================
void readLoadCells() {
  for (int i = 0; i < 2; i++) {
    int raw = analogRead(getSensorPin(i));
    rawReadings[i] = raw;
    calibratedReadings[i] = constrain(map(raw, sensorMin[i], sensorMax[i], 0, 4095), 0, 4095);
  }
}

void calculateWeight() {
  float total = 0.0;
  for (int i = 0; i < 2; i++) {
    individualWeights[i] = calibratedReadings[i] * WEIGHT_CALIBRATION_FACTOR * SENSOR_WEIGHT_FACTORS[i];
    total += individualWeights[i];
  }
  static float smooth = 0.0;
  smooth          = 0.7f * smooth + 0.3f * total;
  estimatedWeight = smooth;
}

// ======================================================
// LIMIT CHECK + VIBRATION
// ======================================================
void checkWeightLimits() {
  bool inStance = (estimatedWeight > PATIENT_BODY_WEIGHT_LBS * 0.02f);
  if (!inStance) {
    digitalWrite(VIBRATOR_PIN, LOW);
    weightWarningActive = false;
    criticalWeightAlert = false;
    return;
  }

  if (estimatedWeight > CRITICAL_WEIGHT_LBS) {
    if (!criticalWeightAlert || (millis() - lastWeightAlert > WEIGHT_ALERT_INTERVAL)) {
      triggerCriticalWeightAlert();
      lastWeightAlert = millis();
    }
    criticalWeightAlert = true;
  } else if (estimatedWeight > MAX_ALLOWED_WEIGHT_LBS) {
    if (!weightWarningActive || (millis() - lastWeightAlert > WEIGHT_ALERT_INTERVAL)) {
      triggerWeightWarning();
      lastWeightAlert = millis();
    }
    weightWarningActive = true;
  } else {
    digitalWrite(VIBRATOR_PIN, LOW);
    weightWarningActive = false;
    criticalWeightAlert = false;
  }
}

void triggerCriticalWeightAlert() {
  Serial.println("!!! CRITICAL: over cap + margin !!!");
  for (int i = 0; i < 3; i++) {
    digitalWrite(VIBRATOR_PIN, HIGH); delay(200);
    digitalWrite(VIBRATOR_PIN, LOW);  delay(200);
  }
}

void triggerWeightWarning() {
  Serial.println("! WARNING: over cap !");
  for (int i = 0; i < 2; i++) {
    digitalWrite(VIBRATOR_PIN, HIGH); delay(400);
    digitalWrite(VIBRATOR_PIN, LOW);  delay(400);
  }
}

// ======================================================
// HELPERS
// ======================================================
int getSensorPin(int index) {
  return (index == 0) ? LOADCELL_1_PIN : LOADCELL_2_PIN;
}

void displayReadings() {
  float pctBW = (estimatedWeight / PATIENT_BODY_WEIGHT_LBS) * 100.0f;
  Serial.print("Total: "); Serial.print(estimatedWeight, 1);
  Serial.print(" lbs ("); Serial.print(pctBW, 1); Serial.print("% BW) | ");
  for (int i = 0; i < 2; i++) {
    Serial.print("L"); Serial.print(i + 1); Serial.print(": ");
    Serial.print(calibratedReadings[i]); Serial.print(" (");
    Serial.print(individualWeights[i], 1); Serial.print(" lbs)  ");
  }
  if      (estimatedWeight > CRITICAL_WEIGHT_LBS)    Serial.print("[CRITICAL]");
  else if (estimatedWeight > MAX_ALLOWED_WEIGHT_LBS)  Serial.print("[WARNING]");
  else                                                 Serial.print("[OK]");
  Serial.println();
}
