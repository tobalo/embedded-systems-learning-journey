# Servo + Ultrasonic Beacon

A servo that tracks distance from an HC-SR04 ultrasonic sensor and beacons its
state over serial as MessagePack. Built on an Arduino Mega 2560 with a
solderless breadboard sharing one 5V / GND rail.

Pipeline: **read distance → filter → map to angle → slew servo → beacon state**

---

## Hardware

| Component | Model | Key specs |
|---|---|---|
| Controller | Arduino Mega 2560 R3 | 5V logic, 54 digital I/O, USB-powered |
| Servo | SG90 | 3.5–6V, 0–180°, ~100 mA idle / 600–700 mA stall, PWM signal |
| Distance sensor | HC-SR04 | 5V, ~15 mA, range 2–400 cm, accuracy ±3 mm, ≥60 ms per ping |
| Smoothing cap | Electrolytic | 100–470 µF, across + and – rails, + leg to red rail |
| Breadboard | Full / half | Solderless, power rails + terminal strips |

---

## Pin Assignments

| Signal | Arduino pin | Goes to |
|---|---|---|
| Servo power | 5V (via + rail) | Servo VCC (red) |
| Servo ground | GND (via – rail) | Servo GND (brown) |
| Servo signal | **D9** | Servo SIG (orange), PWM |
| Sensor power | 5V (via + rail) | HC-SR04 VCC |
| Sensor ground | GND (via – rail) | HC-SR04 GND |
| Sensor trigger | **D12** | HC-SR04 TRIG |
| Sensor echo | **D11** | HC-SR04 ECHO |

> Pins must match the `#define`s in the sketch: `TRIG_PIN 12`, `ECHO_PIN 11`,
> servo on `9`. If you rewire, update the defines.

---

## Breadboard Wiring

The board's power rails act as distribution buses, so one 5V pin powers
everything.

```
                 +5V rail (red)  ────────────────────────────────
                 GND rail (blue) ────────────────────────────────
                   │   │                    │            │
   Arduino 5V ─────┘   │      Servo VCC ────┘   HC VCC ──┘
   Arduino GND ────────┘      Servo GND ───┐   HC GND ──┐
                                           └── GND rail ─┘
   Arduino D9  ───────────────── Servo SIG  (signal, skips rails)
   Arduino D12 ───────────────── HC TRIG    (signal, skips rails)
   Arduino D11 ───────────────── HC ECHO    (signal, skips rails)
   C1 100–470µF  ─── across + and – rails (near the servo)
```

### Rail rules

1. **One strip = one net.** Every hole in a rail is the same node. Plug any
   device leg into any hole on that rail.
2. **Inject once.** A single jumper `5V → + rail` and one `GND → – rail` powers
   the whole strip — that's how one 5V pin feeds multiple devices.
3. **Mind the split.** Many full boards break each rail in the middle. If the
   red/blue line has a gap, bridge it with a jumper or half the rail is dead.
4. **Top ≠ bottom.** The top and bottom rail pairs are separate. Link them with
   jumpers if you use both.
5. **Common ground.** Servo, sensor, Arduino, and any external supply all share
   the – rail. No shared ground → floating signals → garbage readings.
6. **Signals skip the rails.** D9 / D11 / D12 are data, not power. Run them
   pin-to-pin, never into a power rail.
7. **Under load, split power.** If the servo works hard, give it its own 5V
   supply and still tie that supply's ground to the – rail.

---

## Behavior

- **Mapping:** distance 5–50 cm → angle 0–180° (linear, ~4° per cm). Closer than
  5 cm pins at 0°, past 50 cm pins at 180°.
- **Filtering:** exponential moving average (`alpha = 0.30`) plus an
  out-of-range flag, so noise doesn't whip the servo.
- **Motion:** slew-limited to 1° per 15 ms (~66°/s) — smooth, non-blocking.
- **Beacon:** 10 Hz state packet `{t, d, a, g, s}` (time, distance, angle,
  target, status) over serial.

### Serial beacon

Set the mode in the sketch:

- `BEACON_TEXT 1` — readable line + hex dump of the packed bytes (use in the
  Serial Monitor).
- `BEACON_TEXT 0` — raw MessagePack frames: `0xA5` sync byte + length + payload
  (use with a host receiver).

Baud rate: **9600**.

---

## Build & Run

1. Install the **Servo** and **SR04** libraries (Arduino IDE → Library Manager).
2. Wire per the table above; double-check common ground.
3. Open `servo_ultrasonic_beacon.ino`, select **Arduino Mega 2560**, upload.
4. Open Serial Monitor at 9600 baud and move your hand toward the sensor.