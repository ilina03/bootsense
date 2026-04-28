# 👟 Boot Sense

**A sensor-based orthopedic boot for real-time partial weight-bearing monitoring**

> University of Toledo — BIOE 4420 Senior Design Project  
> Team: Hailey Jackson, Ilina Jayal, Raquel Kocaj, Bailee Traver, Shereen Yassine  
> Advisor: Dr. Yuan Tang

---

## 📖 Overview

Boot Sense is a low-cost, sensor-based system that helps patients recovering from lower-limb injuries safely adhere to **partial weight-bearing (PWB)** restrictions while using a walking boot.

Over **62.5% of patients** prescribed weight-bearing limitations unknowingly exceed their allowed load, risking delayed healing or reinjury. Boot Sense addresses this by embedding load sensors directly into a standard medical walking boot and providing **real-time vibration and LED alerts** when prescribed limits are exceeded.

---

## 🔧 Hardware

| Component | Description |
|---|---|
| **Microcontroller** | Arduino MKR WiFi 1010 (SAMD) |
| **Load Cells** | 2× strain-gauge load cells (heel + forefoot) |
| **Amplifier** | HX711 24-bit instrumentation amplifier |
| **Feedback** | Coin vibration motor + LED indicator |
| **Power** | Rechargeable battery pack |

### Wiring

```
Load Cell 1 ──┐
              ├──► HX711 ──► Arduino MKR WiFi 1010 ──► Vibrator Motor (Pin 6)
Load Cell 2 ──┘                                     └──► LED (Pin 7)
```

**Pin Assignments:**
- `DOUT` → A5
- `SCK` → A3
- Vibrator → Digital Pin 6
- LED → Digital Pin 7

---

## ⚙️ How It Works

### Calibration
The system uses a two-point linear calibration:

1. **EMPTY** — raw ADC reading with no weight on the boot
2. **STAND** — raw ADC reading with the patient's full body weight

```
%BW = (reading - EMPTY) / (STAND - EMPTY)
```

This maps sensor output to **percent body weight (%BW)** without requiring external calibration weights.

### Alert Logic (Hysteresis)

To prevent false triggers and "flicker" near the threshold:

| Condition | Action |
|---|---|
| `%BW ≥ 30%` sustained for `≥ 300 ms` | Vibrator + LED **ON** |
| `%BW ≤ 20%` | Vibrator + LED **OFF** |

The hysteresis band (30% on / 20% off) ensures the alert only activates during meaningful load events, not noise.

---

## 💻 Software

### Dependencies
- [HX711 Arduino Library](https://github.com/bogde/HX711) by bogde

### Configuration (edit before flashing)

In `firmware/bootsense_hx711/bootsense_hx711.ino`:

```cpp
// Calibration values — capture these from your own boot/patient
const long EMPTY = -685000;   // raw ADC, no weight
const long STAND = -525000;   // raw ADC, full body weight

// Threshold settings
const float PCT_ON  = 0.30;   // alert ON  at 30% BW
const float PCT_OFF = 0.20;   // alert OFF at 20% BW
const unsigned long HOLD_MS = 300; // ms sustained above threshold to trigger
```

### Flashing
1. Install the [Arduino IDE](https://www.arduino.cc/en/software)
2. Install the **HX711** library via Library Manager
3. Select board: **Arduino MKR WiFi 1010**
4. Open `firmware/bootsense_hx711/bootsense_hx711.ino`
5. Upload to board

---

## 📊 Testing Results

| Test | Result |
|---|---|
| Load detection | ✅ Reliable — stable ADC values after averaging |
| %BW calibration | ✅ Predictable linear scaling across trials |
| Alert activation at 30% BW | ✅ Consistent, no false triggers |
| Hysteresis (off at 20% BW) | ✅ No flicker observed |
| Independent heel/forefoot sensing | ❌ Single HX711 channel — combined signal only |
| Embedded/wearable electronics | ❌ External prototype only |
| Gait (walking) testing | ❌ Not performed — electronics not yet embedded |

---

## 🚧 Known Limitations & Future Work

- [ ] **Dual-channel amplification** — one HX711 per load cell for independent heel/forefoot sensing
- [ ] **Embedded electronics** — integrate PCB and battery into boot shell
- [ ] **Dedicated power supply** — Li-ion + 3.3V LDO regulator (USB power banks are unreliable)
- [ ] **Clinician interface** — keypad, Bluetooth app, or PC utility to adjust thresholds without editing firmware
- [ ] **Gait testing** — walking trials once electronics are safely embedded
- [ ] **Clinical validation** — real-world accuracy and usability studies

---

## 📁 Repo Structure

```
bootsense/
├── firmware/
│   ├── bootsense_hx711/          # Final HX711 + hysteresis version
│   └── bootsense_analog_test/    # Earlier analog ADC test version
├── docs/
│   ├── Final_Report_Boot_Sense.pdf
│   └── Final_Presentation.pptx
├── hardware/
│   ├── wiring_diagram.svg
│   ├── HARDWARE.md
│   └── BOM_bootsense.csv
└── README.md
```

---

## 📄 License

This project was developed for academic purposes at the University of Toledo. All rights reserved by the team unless otherwise noted.

---

## 👥 Team

| Name | Role |
|---|---|
| Hailey Jackson | Senior Bioengineering Student |
| Ilina Jayal | Senior Bioengineering Student |
| Raquel Kocaj | Senior Bioengineering Student |
| Bailee Traver | Senior Bioengineering Student |
| Shereen Yassine | Senior Bioengineering Student |
