#!/usr/bin/env python3
"""
Volvo LIN -> Hudiy bridge for Raspberry Pi.

This script listens to Volvo LIN frames via a USBlini adapter and turns
specific button frames into keyboard events that Hudiy already understands.

Default mapping is based on the LIN table provided by the user:
  frame id 0x01
  data bytes like:
    3F 00  no press
    3D 00  next track
    3E 00  previous track
    37 00  volume up
    3B 00  volume down
    3F 10  back
    3F 20  enter/select
    3F 08  up
    3F 04  down
    3F 02  left
    3F 01  right
    2F 00  answer call
    1F 00  hang up

If your vehicle uses a different frame id, pass --frame-id.
"""

from __future__ import annotations

import argparse
import logging
import signal
import time
from dataclasses import dataclass
from typing import Dict, List, Optional, Sequence, Tuple

from evdev import UInput, ecodes

try:
    from usblini import USBlini, USBliniError
except ImportError:  # pragma: no cover - fallback for some package layouts
    from usblini.usblini import USBlini, USBliniError


LOG = logging.getLogger("volvo-lin-hudiy-bridge")


@dataclass(frozen=True)
class ButtonAction:
    name: str
    keys: Tuple[int, ...]


BUTTON_MAP: Dict[Tuple[int, int], ButtonAction] = {
    (0x3D, 0x00): ButtonAction("next_track", (ecodes.KEY_N,)),
    (0x3E, 0x00): ButtonAction("previous_track", (ecodes.KEY_V,)),
    (0x37, 0x00): ButtonAction("volume_up", (ecodes.KEY_VOLUMEUP,)),
    (0x3B, 0x00): ButtonAction("volume_down", (ecodes.KEY_VOLUMEDOWN,)),
    (0x3F, 0x10): ButtonAction("back", (ecodes.KEY_ESC,)),
    (0x3F, 0x20): ButtonAction("enter", (ecodes.KEY_ENTER,)),
    (0x3F, 0x08): ButtonAction("up", (ecodes.KEY_UP,)),
    (0x3F, 0x04): ButtonAction("down", (ecodes.KEY_DOWN,)),
    (0x3F, 0x02): ButtonAction("left", (ecodes.KEY_LEFT,)),
    (0x3F, 0x01): ButtonAction("right", (ecodes.KEY_RIGHT,)),
    (0x2F, 0x00): ButtonAction("answer_call", (ecodes.KEY_P,)),
    (0x1F, 0x00): ButtonAction("hang_up", (ecodes.KEY_O,)),
}

LONG_PRESS_ACTIONS: Dict[Tuple[int, int], ButtonAction] = {
    (0x3F, 0x02): ButtonAction("scroll_left_1", (ecodes.KEY_1,)),
    (0x3F, 0x01): ButtonAction("scroll_right_2", (ecodes.KEY_2,)),
}

NO_PRESS = (0x3F, 0x00)


def parse_int(value: str) -> int:
    return int(value, 0)


def format_payload(data: Sequence[int]) -> str:
    return " ".join(f"{byte:02X}" for byte in data)


def get_frame_id(frame) -> int:
    for attr in ("frameid", "id", "identifier"):
        if hasattr(frame, attr):
            return int(getattr(frame, attr))
    raise AttributeError("Frame object does not expose a frame id")


def get_frame_data(frame) -> List[int]:
    data = getattr(frame, "data", None)
    if data is None:
        return []
    return [int(x) & 0xFF for x in data]


def create_uinput() -> UInput:
    capabilities = {
        ecodes.EV_KEY: [
            ecodes.KEY_1,
            ecodes.KEY_2,
            ecodes.KEY_N,
            ecodes.KEY_V,
            ecodes.KEY_VOLUMEUP,
            ecodes.KEY_VOLUMEDOWN,
            ecodes.KEY_ESC,
            ecodes.KEY_ENTER,
            ecodes.KEY_UP,
            ecodes.KEY_DOWN,
            ecodes.KEY_LEFT,
            ecodes.KEY_RIGHT,
            ecodes.KEY_P,
            ecodes.KEY_O,
        ],
    }
    return UInput(capabilities, name="volvo-lin-hudiy-bridge")


def tap_key(ui: UInput, key_code: int) -> None:
    ui.write(ecodes.EV_KEY, key_code, 1)
    ui.syn()
    ui.write(ecodes.EV_KEY, key_code, 0)
    ui.syn()


def tap_key_sequence(ui: UInput, key_codes: Sequence[int], gap_s: float = 0.25) -> None:
    for index, key_code in enumerate(key_codes):
        tap_key(ui, key_code)
        if index + 1 < len(key_codes):
            time.sleep(gap_s)


def main() -> int:
    parser = argparse.ArgumentParser(description="Bridge Volvo LIN buttons to Hudiy key events")
    parser.add_argument("--frame-id", default="0x01", help="LIN frame id to watch (default: 0x01)")
    parser.add_argument("--baudrate", default="19200", help="LIN baudrate for USBlini (default: 19200)")
    parser.add_argument("--autobaud", dest="autobaud", action="store_true", help="Enable USBlini autobaud")
    parser.add_argument("--no-autobaud", dest="autobaud", action="store_false", help="Disable USBlini autobaud")
    parser.set_defaults(autobaud=True)
    parser.add_argument("--serial", default=None, help="Optional USBlini serial number")
    parser.add_argument("--repeat-same", action="store_true", help="Repeat actions even if the same frame is held")
    parser.add_argument("--long-press-ms", type=int, default=700, help="Hold duration before long-press actions fire (default: 700)")
    parser.add_argument("--verbose", action="store_true", help="Enable debug logging")
    parser.add_argument("--dump-all-frames", action="store_true", help="Log every received LIN frame, even when unmapped")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    frame_id = parse_int(args.frame_id)
    baudrate = parse_int(args.baudrate)

    stop = False

    def _signal_handler(signum, frame):  # noqa: ARG001
        nonlocal stop
        stop = True
        LOG.info("Stopping...")

    signal.signal(signal.SIGINT, _signal_handler)
    signal.signal(signal.SIGTERM, _signal_handler)

    ui = create_uinput()
    usblini = USBlini()

    active_payload: Optional[Tuple[int, int]] = None
    active_since = 0.0
    long_press_fired = False
    release_timeout_s = 0.20
    long_press_timeout_s = max(0, args.long_press_ms) / 1000.0

    def handle_frame(frame_obj) -> None:
        nonlocal active_payload, active_since, long_press_fired

        try:
            current_frame_id = get_frame_id(frame_obj)
            data = get_frame_data(frame_obj)
        except Exception as exc:  # pragma: no cover - defensive
            LOG.debug("Could not decode frame: %s", exc)
            return

        payload_text = format_payload(data)
        LOG.debug("RX frame 0x%02X: %s", current_frame_id, payload_text)

        if current_frame_id != frame_id:
            return

        if len(data) < 2:
            return

        payload = (data[0], data[1])

        if payload == NO_PRESS:
            active_payload = None
            active_since = 0.0
            long_press_fired = False
            return

        first_seen = payload != active_payload
        if first_seen:
            active_payload = payload
            active_since = time.monotonic()
            long_press_fired = False

        if not args.repeat_same and not first_seen:
            return

        action = BUTTON_MAP.get(payload)
        if action is None:
            LOG.info("Unmapped frame 0x%02X: %s", current_frame_id, payload_text)
            if not args.dump_all_frames:
                return
            LOG.debug("No action mapped for 0x%02X payload %s", current_frame_id, payload_text)
            return

        LOG.info("Action %s from 0x%02X: %s", action.name, current_frame_id, payload_text)
        for key_code in action.keys:
            tap_key(ui, key_code)

    try:
        usblini.open(args.serial)
        usblini.reset()
        usblini.set_baudrate(baudrate, args.autobaud)
        usblini.frame_listener_add(handle_frame)
        LOG.info(
            "Listening on USBlini serial=%s baudrate=%s autobaud=%s frame-id=0x%02X",
            args.serial,
            baudrate,
            args.autobaud,
            frame_id,
        )
        LOG.info("Button map: %s", ", ".join(f"0x{k[0]:02X} 0x{k[1]:02X}->{v.name}" for k, v in BUTTON_MAP.items()))
        LOG.info("Long press map: %s", ", ".join(f"0x{k[0]:02X} 0x{k[1]:02X}->{v.name}" for k, v in LONG_PRESS_ACTIONS.items()))
        LOG.info("Long press timeout: %d ms", args.long_press_ms)

        while not stop:
            time.sleep(0.05)

            if active_payload is not None and not long_press_fired:
                action = LONG_PRESS_ACTIONS.get(active_payload)
                if action is not None and time.monotonic() - active_since >= long_press_timeout_s:
                    LOG.info("Long press %s", action.name)
                    tap_key_sequence(ui, action.keys)
                    long_press_fired = True
    except USBliniError as exc:
        LOG.error("USBlini error: %s", exc)
        return 2
    finally:
        try:
            usblini.close()
        except Exception:
            pass
        ui.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
