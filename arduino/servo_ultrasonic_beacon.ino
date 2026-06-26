/*
 * servo_ultrasonic_beacon.ino
 * Fuses Lesson 9 (Servo) + Lesson 10 (HC-SR04) into one non-blocking sketch.
 *
 * Pipeline:  read distance -> filter -> map to target angle -> slew servo
 *            -> beacon state as a MessagePack frame
 *
 * Wiring (shared rails): servo VCC/GND on the rails, SIG -> D9.
 *   HC-SR04 VCC/GND on the rails, TRIG -> D12, ECHO -> D11. Common ground.
 */
 
#include <Servo.h>
#include "SR04.h"
 
#define TRIG_PIN 12
#define ECHO_PIN 11
#define SERVO_PIN 9
 
// ---- beacon mode: 1 = human-readable text (use in Serial Monitor)
//                   0 = raw MessagePack binary frames (use with a real receiver)
#define BEACON_TEXT 1
 
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
Servo myservo;
 
// ---- tuning ----
const int   DIST_NEAR = 5;     // cm -> angle 0
const int   DIST_FAR  = 50;    // cm -> angle 180
const float EMA_ALPHA = 0.30;  // distance smoothing (lower = smoother/slower)
const unsigned long SAMPLE_MS = 60;   // HC-SR04 needs >=60ms between pings
const unsigned long STEP_MS   = 15;   // servo slew: one degree per step
const unsigned long BEACON_MS = 100;  // 10 Hz state beacon
 
// ---- shared state (this IS the message pack payload) ----
struct State {
  uint32_t t_ms;     // timestamp
  int16_t  dist_cm;  // filtered distance
  int16_t  angle;    // current servo angle
  int16_t  target;   // target servo angle
  uint8_t  status;   // 0 = ok, 1 = out of range
};
State st;
 
float distFilt = 50.0;
int   current  = 90;   // current servo angle
int   target   = 90;   // target servo angle
unsigned long tSample = 0, tStep = 0, tBeacon = 0;
 
void setup() {
  Serial.begin(9600);
  myservo.attach(SERVO_PIN);
  myservo.write(current);
  delay(500);
}
 
void loop() {
  unsigned long now = millis();
 
  if (now - tSample >= SAMPLE_MS) { tSample = now; sample(); }
  if (now - tStep   >= STEP_MS)   { tStep   = now; slew();   }
  if (now - tBeacon >= BEACON_MS) { tBeacon = now; state(); beacon(); }
}
 
// --- read sensor, filter, map to target angle ---
void sample() {
  long raw = sr04.Distance();                 // cm (blocking ping)
  st.status = (raw < 2 || raw > 400) ? 1 : 0; // flag bad reads
  if (st.status == 0) {
    distFilt = EMA_ALPHA * raw + (1.0 - EMA_ALPHA) * distFilt;  // smooth jitter
    int d = constrain((int)distFilt, DIST_NEAR, DIST_FAR);
    target = map(d, DIST_NEAR, DIST_FAR, 0, 180);
  }
}
 
// --- move one degree toward target (non-blocking) ---
void slew() {
  if (current != target) {
    current += (target > current) ? 1 : -1;
    myservo.write(current);
  }
}
 
// --- gather the current state into the packet struct ---
void state() {
  st.t_ms    = millis();
  st.dist_cm = (int16_t)distFilt;
  st.angle   = current;
  st.target  = target;
}
 
// =================== MessagePack encoder ===================
// Encodes st as a 5-entry map: {t:u32, d:i16, a:i16, g:i16, s:uint}
uint8_t mp[48];
uint8_t n;
 
void mp_str(const char* s) { uint8_t l = strlen(s); mp[n++] = 0xA0 | l; while (*s) mp[n++] = *s++; }
void mp_u32(uint32_t v)    { mp[n++] = 0xCE; mp[n++] = v >> 24; mp[n++] = v >> 16; mp[n++] = v >> 8; mp[n++] = v; }
void mp_i16(int16_t v)     { mp[n++] = 0xD1; mp[n++] = (v >> 8) & 0xFF; mp[n++] = v & 0xFF; }
void mp_fixint(uint8_t v)  { mp[n++] = v & 0x7F; }  // positive fixint, 0..127
 
void pack() {
  n = 0;
  mp[n++] = 0x85;                       // fixmap, 5 entries
  mp_str("t"); mp_u32(st.t_ms);
  mp_str("d"); mp_i16(st.dist_cm);
  mp_str("a"); mp_i16(st.angle);
  mp_str("g"); mp_i16(st.target);
  mp_str("s"); mp_fixint(st.status);
}
 
// --- transmit the state in realtime ---
void beacon() {
  pack();
#if BEACON_TEXT
  // readable in Serial Monitor + hex view of the actual packed frame
  Serial.print("t="); Serial.print(st.t_ms);
  Serial.print(" d="); Serial.print(st.dist_cm); Serial.print("cm");
  Serial.print(" a="); Serial.print(st.angle);
  Serial.print(" g="); Serial.print(st.target);
  Serial.print(" s="); Serial.print(st.status);
  Serial.print("  | msgpack[");
  Serial.print(n); Serial.print("B]: ");
  for (uint8_t i = 0; i < n; i++) {
    if (mp[i] < 0x10) Serial.print('0');
    Serial.print(mp[i], HEX); Serial.print(' ');
  }
  Serial.println();
#else
  // framed binary beacon: 0xA5 sync + length + payload
  Serial.write(0xA5);
  Serial.write(n);
  Serial.write(mp, n);
#endif
}

