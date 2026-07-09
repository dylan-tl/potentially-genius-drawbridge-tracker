import json
import time
import urllib.request

from arduino.app_utils import App, Bridge, Logger

# --- The detection UNO Q (serves GET /bridge on its WebUI port) ---
CLASSIFIER_HOST = "192.168.2.91"  # IP of the detection UNO Q
CLASSIFIER_PORT = 7000
STATE_URL = f"http://{CLASSIFIER_HOST}:{CLASSIFIER_PORT}/bridge"

POLL_INTERVAL_SEC = 2.0
HTTP_TIMEOUT_SEC = 3.0
# The polled state must differ from the last commanded state continuously for
# this long before the model moves, so a momentary misdetection never drives
# the servos. Any poll failure, unknown state, or flap-back resets the clock.
DEBOUNCE_SEC = 5.0

logger = Logger("BridgeScaleModel")

# Last state successfully acked by the sketch. None = never commanded, so the
# first stable reading after any restart always drives the model.
last_commanded: bool | None = None
candidate: bool | None = None         # differing state currently being timed
candidate_since: float | None = None  # time.monotonic() when candidate first seen


def reset_candidate():
    global candidate, candidate_since
    candidate = None
    candidate_since = None


def poll_and_mirror():
    global last_commanded, candidate, candidate_since

    try:
        with urllib.request.urlopen(STATE_URL, timeout=HTTP_TIMEOUT_SEC) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
    except Exception as e:
        logger.warning(f"Classifier unreachable at {STATE_URL} ({e}); holding last state")
        reset_candidate()
        return

    is_up = payload.get("is_up")
    if is_up is None:
        # Classifier is up but hasn't classified a frame yet: unknown, do nothing.
        reset_candidate()
        return

    is_up = bool(is_up)
    if is_up == last_commanded:
        reset_candidate()
        return

    now = time.monotonic()
    if candidate != is_up:
        candidate, candidate_since = is_up, now
        logger.info(f"State change to {'UP' if is_up else 'DOWN'} pending ({DEBOUNCE_SEC:.0f}s debounce)")
        return
    if now - candidate_since < DEBOUNCE_SEC:
        return

    try:
        Bridge.call("set_bridge_state", is_up, timeout=5)
        last_commanded = is_up  # only after a successful ack
        reset_candidate()
        logger.info(f"Scale model commanded {'UP' if is_up else 'DOWN'}")
    except Exception as e:
        # last_commanded not updated -> retried on the next poll
        logger.error(f"Bridge call to sketch failed: {e}")


def loop():
    poll_and_mirror()
    time.sleep(POLL_INTERVAL_SEC)


App.run(user_loop=loop)
