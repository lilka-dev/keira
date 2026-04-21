"""
UART Button Emulator client for Keira firmware tests.

Communicates with a device flashed with the `v2_emulator` PlatformIO environment
(-DLILKA_UART_BUTTON_EMULATOR=1).  The device must be connected via USB-CDC.

Protocol (newline-terminated ASCII commands):
  Host → Device:
    PING               → device replies  PONG
    REBOOT             → device replies  REBOOTING  then restarts
    BTN:<NAME>         → pulse-press for 100 ms, replies BTN_OK:<NAME>
    BTN_HOLD:<NAME>    → hold until released, replies BTN_HOLD_OK:<NAME>
    BTN_RELEASE:<NAME> → release a held button, replies BTN_RELEASE_OK:<NAME>

  Button names: UP DOWN LEFT RIGHT A B C D SELECT START
"""

import re
import time
from typing import Optional

import serial


class EmulatorError(Exception):
    pass


class TimeoutError(EmulatorError):
    pass


class KeiraEmulator:
    """Context-manager-friendly client for the Keira UART button emulator."""

    BUTTONS = {"UP", "DOWN", "LEFT", "RIGHT", "A", "B", "C", "D", "SELECT", "START"}

    def __init__(self, port: str, baud: int = 115200, timeout: float = 10.0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self._ser: Optional[serial.Serial] = None

    # ------------------------------------------------------------------
    # Connection lifecycle
    # ------------------------------------------------------------------

    def connect(self) -> None:
        self._ser = serial.Serial(self.port, self.baud, timeout=0.1)
        # Discard stale data
        time.sleep(0.2)
        self._ser.reset_input_buffer()
        self._wait_for("EMULATOR:READY")

    def close(self) -> None:
        if self._ser and self._ser.is_open:
            self._ser.close()

    def __enter__(self) -> "KeiraEmulator":
        self.connect()
        return self

    def __exit__(self, *_) -> None:
        self.close()

    # ------------------------------------------------------------------
    # Public commands
    # ------------------------------------------------------------------

    def ping(self) -> None:
        """Verify the device is alive."""
        self._send("PING")
        self._wait_for("PONG")

    def press(self, button: str) -> None:
        """Pulse-press a button for 100 ms."""
        self._validate_button(button)
        self._send(f"BTN:{button.upper()}")
        self._wait_for(f"BTN_OK:{button.upper()}")

    def hold(self, button: str) -> None:
        """Start holding a button until release() is called."""
        self._validate_button(button)
        self._send(f"BTN_HOLD:{button.upper()}")
        self._wait_for(f"BTN_HOLD_OK:{button.upper()}")

    def release(self, button: str) -> None:
        """Release a previously held button."""
        self._validate_button(button)
        self._send(f"BTN_RELEASE:{button.upper()}")
        self._wait_for(f"BTN_RELEASE_OK:{button.upper()}")

    def reboot(self) -> None:
        """Reboot the device and wait for it to come back up."""
        self._send("REBOOT")
        self._wait_for("REBOOTING")
        # Device restarts — give it time to boot, then wait for the ready banner
        time.sleep(3.0)
        if self._ser:
            self._ser.reset_input_buffer()
        self._wait_for("EMULATOR:READY")

    def wait_for_line(self, pattern: str, timeout: Optional[float] = None) -> str:
        """Block until a line matching *pattern* (regex) arrives from the device."""
        return self._wait_for(pattern, timeout=timeout, regex=True)

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _validate_button(self, button: str) -> None:
        if button.upper() not in self.BUTTONS:
            raise ValueError(
                f"Unknown button '{button}'. Valid buttons: {sorted(self.BUTTONS)}"
            )

    def _send(self, cmd: str) -> None:
        assert self._ser, "Not connected — call connect() first"
        self._ser.write((cmd + "\n").encode("utf-8"))
        self._ser.flush()

    def _wait_for(
        self,
        expected: str,
        timeout: Optional[float] = None,
        regex: bool = False,
    ) -> str:
        assert self._ser, "Not connected — call connect() first"
        deadline = time.monotonic() + (timeout if timeout is not None else self.timeout)
        while time.monotonic() < deadline:
            raw = self._ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="replace").strip()
            if line:
                print(f"[DEVICE] {line}")
            match = re.search(expected, line) if regex else (expected in line)
            if match:
                return line
        raise TimeoutError(
            f"Timed out ({self.timeout}s) waiting for: {expected!r}"
        )
