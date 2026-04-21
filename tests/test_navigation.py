"""
Basic Keira firmware tests using the UART button emulator.

Build the firmware with:
    pio run -e v2_emulator

Flash it, then run:
    pytest tests/test_navigation.py --port /dev/ttyACM0 -v
"""

import time

import pytest


# ---------------------------------------------------------------------------
# Connectivity
# ---------------------------------------------------------------------------


def test_ping(keira):
    """Device is reachable and the emulator is running."""
    keira.ping()


# ---------------------------------------------------------------------------
# Menu navigation
# ---------------------------------------------------------------------------


def test_navigate_down_and_up(keira):
    """DOWN then UP should leave the selection where it started."""
    keira.press("DOWN")
    time.sleep(0.3)
    keira.press("UP")
    time.sleep(0.3)


def test_navigate_full_cycle(keira):
    """Navigate through several items and return."""
    for _ in range(5):
        keira.press("DOWN")
        time.sleep(0.2)
    for _ in range(5):
        keira.press("UP")
        time.sleep(0.2)


# ---------------------------------------------------------------------------
# App entry / exit
# ---------------------------------------------------------------------------


def test_open_and_close_app(keira):
    """A (select) should open an app; B should exit back to the menu."""
    keira.press("A")
    time.sleep(0.5)
    keira.press("B")
    time.sleep(0.3)


def test_open_nested_app_and_back(keira):
    """Navigate into a submenu item and come back with two B presses."""
    keira.press("DOWN")
    time.sleep(0.2)
    keira.press("A")
    time.sleep(0.5)
    keira.press("A")
    time.sleep(0.5)
    keira.press("B")
    time.sleep(0.3)
    keira.press("B")
    time.sleep(0.3)


# ---------------------------------------------------------------------------
# Hold / release
# ---------------------------------------------------------------------------


def test_hold_and_release(keira):
    """Hold DOWN for 600 ms to scroll through multiple items."""
    keira.hold("DOWN")
    time.sleep(0.6)
    keira.release("DOWN")
    time.sleep(0.2)


# ---------------------------------------------------------------------------
# Reboot
# ---------------------------------------------------------------------------


@pytest.mark.last
def test_reboot(keira):
    """Device reboots cleanly and the emulator comes back up."""
    keira.reboot()
    keira.ping()
