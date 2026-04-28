# Hardware

This folder contains hardware documentation for the Boot Sense prototype.

## Contents

| File | Description |
|---|---|
| `BOM_bootsense.csv` | Bill of materials with component specs, costs, and sources |
| `wiring_diagram.svg` | Schematic showing all electrical connections |
| `photos/` | Prototype photos — add yours here (see below) |

---

## Wiring Summary

```
Load Cell 1 (heel)    ──┐
                        ├──► HX711 (3.3V!) ──► Arduino MKR WiFi 1010
Load Cell 2 (forefoot)──┘      DOUT → A5
                               SCK  → A3

Arduino MKR WiFi 1010 ──► Pin 6 → NPN transistor → Coin vibrator motor
                      └──► Pin 7 → 220Ω resistor  → LED (to GND)
```

### Critical Notes
- **Power the HX711 at 3.3V only** — using the 5V pin causes unstable ADC readings and can permanently damage the HX711 module.
- **Do not connect the vibration motor directly to a GPIO pin.** Use an NPN transistor (e.g. 2N2222) or a small motor driver to handle the motor's current draw, which can cause voltage dips that affect sensor readings.
- **USB power banks are unreliable** for this circuit — they are designed for phone charging and can cut out or fluctuate at microcontroller-level loads. A dedicated Li-ion cell with a 3.3V LDO regulator is the recommended upgrade for future versions.

---

## Load Cell Placement

Load cells are positioned **underneath the boot frame**:
- **Load cell 1** — heel region
- **Load cell 2** — forefoot/metatarsal region

Both are wired in parallel to a single HX711 channel, giving a combined load reading. Future versions should use two HX711 modules (one per cell) for independent heel and forefoot sensing.

---

## Adding Photos

Drag and drop prototype photos into the `hardware/photos/` folder directly on GitHub. Suggested shots:
- Full boot with electronics attached (top view)
- Underside of boot showing load cell placement
- Electronics breadboard close-up
- HX711 wiring to Arduino
- Boot being worn / demo in use

---

## Known Hardware Issues

| Issue | Impact | Recommended Fix |
|---|---|---|
| Both load cells share one HX711 channel | Cannot measure heel vs forefoot independently | Add a second HX711 module |
| Electronics mounted externally | Not wearable; no gait testing possible | Embed in sealed boot-shell module |
| USB battery pack power | Voltage instability; sensor noise | Dedicated Li-ion + 3.3V LDO regulator |
| Breadboard wiring | Not durable; wires pull loose | Custom PCB |
