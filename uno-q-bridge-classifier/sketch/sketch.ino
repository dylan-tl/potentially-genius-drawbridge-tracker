#include <Arduino_RouterBridge.h>
#include <Arduino_LED_Matrix.h>

Arduino_LED_Matrix matrix;

// Frames exported from the Arduino LED Matrix editor: row-major 8 rows x 13
// columns, brightness 0-7 (rendered with setGrayscaleBits(3)).
const uint8_t ANIM_FRAME_COUNT = 8;
const uint8_t UP_ANIM[ANIM_FRAME_COUNT][104] = {
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,
    0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
};

const uint8_t DOWN_FRAME[104] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0,
  0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7,
  0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0
};

const unsigned long FRAME_INTERVAL_MS = 150;  // UP animation frame period

// -1 = unknown (blank), 0 = down (static image), 1 = up (looping animation).
// Written only by the Bridge handler, read by loop().
int displayTarget = -1;
int displayed = -2;  // sentinel so the first loop() pass always paints
uint8_t frameIdx = 0;
unsigned long lastFrameAt = 0;

// RPC from the Linux side (Bridge.call("set_display_state", bool)). Registered
// with provide_safe so it runs in loop() context. Repeated identical calls
// (the Python resync loop) are no-ops for the state machine, so they never
// restart the animation.
bool set_display_state(bool up) {
  displayTarget = up ? 1 : 0;
  return true;
}

void setup() {
  matrix.begin();
  matrix.setGrayscaleBits(3);  // frames use brightness 0-7
  matrix.clear();              // boot state: unknown -> blank
  Bridge.begin();
  Bridge.provide_safe("set_display_state", set_display_state);
}

void loop() {
  bool entered = (displayTarget != displayed);

  if (displayTarget == -1) {
    if (entered) matrix.clear();
  } else if (displayTarget == 0) {
    if (entered) matrix.draw(DOWN_FRAME);  // draw() persists; paint once
  } else {
    if (entered) {
      frameIdx = 0;
      lastFrameAt = 0;  // draw the first frame immediately
    }
    if (millis() - lastFrameAt >= FRAME_INTERVAL_MS) {
      matrix.draw(UP_ANIM[frameIdx]);
      frameIdx = (frameIdx + 1) % ANIM_FRAME_COUNT;
      lastFrameAt = millis();
    }
  }
  displayed = displayTarget;
}
