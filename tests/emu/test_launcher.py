import os
import struct
import time

from conftest import OUT_DIR


def png_size(data):
    assert data[:8] == b"\x89PNG\r\n\x1a\n", "not a PNG"
    return struct.unpack(">II", data[16:24])


def test_screenshot_is_whole_screen(emu):
    # Lilka v2 display is 280x240 in landscape
    assert png_size(emu.screenshot()) == (280, 240)


def test_buttons_change_screen(emu):
    time.sleep(2)  # let the launcher settle
    before = emu.screenshot(os.path.join(OUT_DIR, "launcher_before.png"))
    emu.press("down")
    time.sleep(1)
    after = emu.screenshot(os.path.join(OUT_DIR, "launcher_after.png"))
    assert before != after, "pressing down did not change the screen"
