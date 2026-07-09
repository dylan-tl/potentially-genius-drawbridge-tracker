#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino_RouterBridge.h>
// --- Servos ---
Servo myservo;
Servo myservotwo;
const int S1_UP = 150, S1_DOWN = 20;
const int S2_UP = 50,   S2_DOWN = 160;
;
// --- NeoPixel ---
#define PIN        8
#define NUM_PIXELS 7
Adafruit_NeoPixel jewel(NUM_PIXELS, PIN, NEO_GRBW + NEO_KHZ800);
const uint8_t W_R = 255;
const uint8_t W_G = 100;
const uint8_t W_B = 0;
const uint8_t W_W = 255;
// --- Timing ---
const unsigned long SERVO_DURATION = 2000;
const unsigned long FADE_DURATION  = 2000;
String inputString = "";
enum State { IDLE, MOVING_SERVOS, FADING_LIGHT };
State state = IDLE;
// Last commanded direction (1 = up, 0 = down, -1 = unknown at boot). Shared by
// the Serial and RPC paths so a repeated command doesn't re-run the animation.
int lastTarget = -1;
unsigned long phaseStart = 0;
float s1Start, s1End, s1Pos;
float s2Start, s2End, s2Pos;
float brightStart, brightEnd, brightPos;
void setAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    jewel.setPixelColor(i, jewel.Color(r, g, b, w));
  }
  jewel.show();
}
void applyBrightness(float b) {
  int level = constrain((int)(b + 0.5f), 0, 255);
  setAll(
    (W_R * level) / 255,
    (W_G * level) / 255,
    (W_B * level) / 255,
    (W_W * level) / 255
  );
}
// RPC handler for the Linux side (Bridge.call("set_bridge_state", bool)).
// Registered with provide_safe so it runs in the loop() context — it touches
// Servo and Serial. Returns an ack so Python can tell delivered from lost.
bool set_bridge_state(bool up) {
  if (state == IDLE && lastTarget == (up ? 1 : 0)) {
    return true;  // already there; skip the servo re-attach
  }
  lastTarget = up ? 1 : 0;
  startCommand(up);
  return true;
}

void setup() {
  Serial.begin(115200);
  Bridge.begin();
  Bridge.provide_safe("set_bridge_state", set_bridge_state);
  jewel.begin();
  jewel.setBrightness(255);
  jewel.show();
  s1Pos = S1_DOWN;
  s2Pos = S2_DOWN;
  brightPos = 0;
  myservo.attach(9);
  myservotwo.attach(11);
  myservo.write(S1_DOWN);
  myservotwo.write(S2_DOWN);
  delay(500);
  myservo.detach();
  myservotwo.detach();
  Serial.println("Starting DOWN. Send UP or DOWN.");
}
void startCommand(bool goingUp) {
  myservo.attach(9);
  myservotwo.attach(11);
  s1Start = s1Pos;
  s2Start = s2Pos;
  brightStart = brightPos;
  s1End = goingUp ? S1_UP : S1_DOWN;
  s2End = goingUp ? S2_UP : S2_DOWN;
  brightEnd = goingUp ? 255 : 0;
  phaseStart = millis();
  state = MOVING_SERVOS;
  Serial.println(goingUp ? "Servos moving UP..." : "Servos moving DOWN...");
}
void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputString.trim();
      inputString.toUpperCase();
      if (inputString == "UP") {
        lastTarget = 1;
        startCommand(true);
      } else if (inputString == "DOWN") {
        lastTarget = 0;
        startCommand(false);
      }
      inputString = "";
    } else {
      inputString += c;
    }
  }
  if (state == MOVING_SERVOS) {
    float t = (float)(millis() - phaseStart) / (float)SERVO_DURATION;
    if (t >= 1.0f) t = 1.0f;
    s1Pos = s1Start + (s1End - s1Start) * t;
    s2Pos = s2Start + (s2End - s2Start) * t;
    myservo.write((int)(s1Pos + 0.5f));
    myservotwo.write((int)(s2Pos + 0.5f));
    if (t >= 1.0f) {
      delay(50);
      myservo.detach();
      myservotwo.detach();
      Serial.println("Servos done. Fading light...");
      phaseStart = millis();
      state = FADING_LIGHT;
    }
  }
  if (state == FADING_LIGHT) {
    float t = (float)(millis() - phaseStart) / (float)FADE_DURATION;
    if (t >= 1.0f) t = 1.0f;
    brightPos = brightStart + (brightEnd - brightStart) * t;
    applyBrightness(brightPos);
    if (t >= 1.0f) {
      state = IDLE;
      Serial.println("Done.");
    }
  }
}