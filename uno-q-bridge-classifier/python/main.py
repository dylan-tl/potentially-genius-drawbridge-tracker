# SPDX-FileCopyrightText: Copyright (C) Arduino s.r.l. and/or its affiliated companies
#
# SPDX-License-Identifier: MPL-2.0

from arduino.app_utils import App, Bridge, Logger
from arduino.app_bricks.web_ui import WebUI
from arduino.app_bricks.video_imageclassification import VideoImageClassification
from arduino.app_peripherals.camera import Camera
from datetime import datetime, UTC
import json
import threading
import time

# Confidence above which the model's UP signal means the bridge is actually up.
# Determined empirically; DOWN's confidence is not part of the trigger.
UP_THRESHOLD = 0.10

# How often the current state is re-pushed to the MCU LED matrix. Heals MCU
# resets, Python-after-sketch startup ordering, and lost calls; re-sends are
# idempotent on the sketch side.
DISPLAY_RESYNC_SEC = 5.0
BRIDGE_CALL_TIMEOUT_SEC = 2

logger = Logger("BridgeClassifier")
ui = WebUI()

# Bridge.call is invoked from both the detection callback and the resync loop.
_bridge_lock = threading.Lock()

# fps capped well below the brick default (10) so the MJPEG preview at :4912/embed
# stays in sync with realtime instead of buffering. Bridge state changes on minute
# timescales — frame rate is not a bottleneck for inference quality.
#
# confidence kept near zero so every classification reaches our callback and shows
# up in the Recent Detections list; bridge-state thresholding happens in Python
# against UP_THRESHOLD. 0.01 (not 0.0) dodges the brick's falsy-check inside
# _override_threshold, which the brick calls on every websocket reconnect.
detection_stream = VideoImageClassification(
    camera=Camera(fps=5),
    confidence=0.01,
    debounce_sec=0.0,
)

# None until the first classification arrives, then True/False. Acts as both the
# authoritative state and the "have we ever published?" flag so the first message
# is always emitted to any subscriber that connects mid-stream.
is_bridge_up: bool | None = None

# ISO timestamp of the most recent inference; lets /bridge consumers spot staleness.
last_classified_at: str | None = None


def push_display_state():
  """Mirror is_bridge_up onto the MCU LED matrix. Failures are logged and
  retried by the periodic resync; never allowed to break the UI path."""
  if is_bridge_up is None:
    return
  try:
    with _bridge_lock:
      Bridge.call("set_display_state", is_bridge_up, timeout=BRIDGE_CALL_TIMEOUT_SEC)
  except Exception as e:
    logger.warning(f"LED matrix sync failed: {e}")


def send_detections_to_ui(classifications: dict):
  if len(classifications) == 0:
    return

  entries = []
  for key, value in classifications.items():
    entry = {
      "content": key,
      "confidence": value,
      "timestamp": datetime.now(UTC).isoformat()
    }
    entries.append(entry)

  if len(entries) > 0:
    msg = json.dumps(entries)
    ui.send_message("classifications", message=msg)

  global is_bridge_up, last_classified_at
  last_classified_at = datetime.now(UTC).isoformat()
  new_state = classifications.get("UP", 0.0) >= UP_THRESHOLD
  if new_state != is_bridge_up:
    logger.info(f"Bridge state: {'UP' if new_state else 'DOWN'}")
    ui.send_message("bridge_state", message=json.dumps({"is_up": new_state}))
    is_bridge_up = new_state
    push_display_state()


detection_stream.on_detect_all(send_detections_to_ui)


def get_bridge_state() -> dict:
  """Polled by the scale-model UNO Q. is_up is null until the first classification."""
  return {"is_up": is_bridge_up, "updated_at": last_classified_at}


ui.expose_api("GET", "/bridge", get_bridge_state)


def display_resync_loop():
  push_display_state()
  time.sleep(DISPLAY_RESYNC_SEC)


App.run(user_loop=display_resync_loop)
