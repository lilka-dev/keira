#!/usr/bin/env python3
"""Run Keira in Espressif's QEMU and drive it over the emulated serial port.

Talks the UARTInputService protocol (src/services/uartinput/uartinput.h): buttons are pressed with
"@press"/"@hold"/"@release", "@screenshot" returns what is on the screen as a PNG.

As a library (for tests):

    with KeiraEmu("build-lilka_v2_qemu-docker/flash_image.bin") as emu:
        emu.wait_for("Launcher")
        emu.press("down")
        emu.screenshot("menu.png")

As a CLI, commands come from a script file or stdin (one per line, see COMMANDS):

    tools/emu/keira_emu.py --image build-lilka_v2_qemu-docker/flash_image.bin script.txt
"""

import argparse
import base64
import os
import re
import shlex
import subprocess
import sys
import threading
import time

BUTTONS = ("up", "down", "left", "right", "a", "b", "c", "d", "select", "start")

# lilka::serial prefixes every line with "[ <millis> ][ L  ] " (or E / I for errors and IDF logs)
LOG_PREFIX = re.compile(r"^\[ \d+ \]\[ (?:\x1b\[[\d;]*m)?([LEI])\s*(?:\x1b\[[\d;]*m)?\s*\] ?")

COMMANDS = """\
press <button> [ms]      press and release a button (buttons: {buttons})
hold <button>            press and keep a button pressed
release [button]         release a button, or all of them
screenshot <file.png>    save what is on the screen
wait <regex> [seconds]   wait until a serial line matches
sleep <seconds>          pause
send <line>              send a raw line to the serial port
quit                     stop the emulator
""".format(buttons=", ".join(BUTTONS))


class EmuError(Exception):
    pass


class KeiraEmu:
    def __init__(self, image, qemu="qemu-system-xtensa", log=None, boot_timeout=60):
        self.image = image
        self.qemu = qemu
        self.log = log
        self.boot_timeout = boot_timeout
        self.proc = None
        self.lines = []
        self.cond = threading.Condition()
        self.command_lock = threading.Lock()
        self.reader = None

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *exc):
        self.stop()

    def start(self):
        if not os.path.isfile(self.image):
            raise EmuError(f"flash image not found: {self.image} (run make qemu-image)")
        # 8 MB of PSRAM like Lilka v2, but quad: octal PSRAM init hangs in QEMU (see boards/lilka_v2_qemu)
        args = [
            self.qemu,
            "-machine", "esp32s3",
            "-m", "8M",
            "-drive", f"file={self.image},if=mtd,format=raw",
            "-display", "none",
            "-monitor", "none",
            "-serial", "stdio",
            "-no-reboot",
        ]  # fmt: skip
        try:
            self.proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        except FileNotFoundError:
            raise EmuError(f"QEMU not found: {self.qemu} (run make qemu-install)") from None
        self.reader = threading.Thread(target=self._read, daemon=True)
        self.reader.start()
        # UARTInputService is one of the last things to start, so its first reply means Keira is up
        deadline = time.monotonic() + self.boot_timeout
        while True:
            try:
                self._command("@release", timeout=1)
                return
            except EmuError:
                if self.proc.poll() is not None:
                    raise EmuError(f"QEMU exited with code {self.proc.returncode} during boot")
                if time.monotonic() > deadline:
                    raise EmuError(f"Keira did not boot in {self.boot_timeout} s")

    def stop(self):
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        if self.reader:
            self.reader.join(1)

    def _read(self):
        buffer = b""
        while True:
            chunk = self.proc.stdout.read1(4096)
            if not chunk:
                break
            buffer += chunk
            *complete, buffer = buffer.split(b"\n")
            for raw in complete:
                line = raw.decode("utf-8", "replace").rstrip("\r")
                if self.log:
                    self.log.write(line + "\n")
                    self.log.flush()
                with self.cond:
                    self.lines.append(line)
                    self.cond.notify_all()
        with self.cond:
            self.cond.notify_all()

    @staticmethod
    def _message(line):
        """Strip lilka::serial's prefix, return the message or None for lines without it."""
        match = LOG_PREFIX.match(line)
        return line[match.end() :] if match else None

    def _wait_line(self, start, predicate, timeout):
        """Wait for a line at index >= start matching predicate, return (index, line)."""
        deadline = time.monotonic() + timeout
        index = start
        with self.cond:
            while True:
                while index < len(self.lines):
                    if predicate(self.lines[index]):
                        return index, self.lines[index]
                    index += 1
                remaining = deadline - time.monotonic()
                if remaining <= 0 or self.proc.poll() is not None:
                    raise EmuError("timed out" if remaining <= 0 else "QEMU exited")
                self.cond.wait(remaining)

    def send(self, line):
        self.proc.stdin.write(line.encode() + b"\n")
        self.proc.stdin.flush()

    def _command(self, line, timeout=10):
        """Send a "@..." command and wait for its "@ok"; return the index of the first line after it was sent."""
        with self.command_lock:
            with self.cond:
                start = len(self.lines)
            self.send(line)
            command = line[1:]

            # "@ok <command>" or "@err <reason>: <command>"
            def reply(text):
                message = self._message(text) or ""
                return message == f"@ok {command}" or message.startswith("@err") and message.endswith(f": {command}")

            try:
                _, text = self._wait_line(start, reply, timeout)
            except EmuError as e:
                raise EmuError(f"{line}: {e}") from None
            if self._message(text).startswith("@err"):
                raise EmuError(self._message(text))
            return start

    def press(self, button, ms=None):
        self._command(f"@press {button}" + (f" {ms}" if ms else ""))

    def hold(self, button):
        self._command(f"@hold {button}")

    def release(self, button=None):
        self._command("@release" + (f" {button}" if button else ""))

    def screenshot(self, path=None, timeout=30):
        """Return what is on the screen as PNG bytes, and save it to path if given."""
        start = self._command("@screenshot", timeout)
        data = b""
        index = start
        while True:
            index, line = self._wait_line(index, lambda text: (self._message(text) or "").startswith("@png "), 1)
            payload = self._message(line)[len("@png ") :]
            index += 1
            if payload.startswith("end "):
                size = int(payload[len("end ") :])
                break
            data += base64.b64decode(payload)
        if len(data) != size:
            raise EmuError(f"screenshot: got {len(data)} bytes, expected {size}")
        if path:
            with open(path, "wb") as f:
                f.write(data)
        return data

    def wait_for(self, pattern, timeout=30, since_start=False):
        """Wait for a serial line matching the regex (only new lines, unless since_start)."""
        regex = re.compile(pattern)
        with self.cond:
            start = 0 if since_start else len(self.lines)
        try:
            return self._wait_line(start, lambda text: regex.search(text), timeout)[1]
        except EmuError as e:
            raise EmuError(f"wait {pattern!r}: {e}") from None


def run_command(emu, line):
    """Run one CLI command, return False on quit."""
    words = shlex.split(line, comments=True)
    if not words:
        return True
    name, args = words[0], words[1:]
    if name == "press" and 1 <= len(args) <= 2:
        emu.press(args[0], int(args[1]) if len(args) > 1 else None)
    elif name == "hold" and len(args) == 1:
        emu.hold(args[0])
    elif name == "release" and len(args) <= 1:
        emu.release(args[0] if args else None)
    elif name == "screenshot" and len(args) == 1:
        emu.screenshot(args[0])
    elif name == "wait" and 1 <= len(args) <= 2:
        emu.wait_for(args[0], float(args[1]) if len(args) > 1 else 30)
    elif name == "sleep" and len(args) == 1:
        time.sleep(float(args[0]))
    elif name == "send" and args:
        emu.send(line.split(None, 1)[1])
    elif name == "quit" and not args:
        return False
    else:
        raise EmuError(f"bad command: {line.strip()}\n{COMMANDS}")
    return True


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        epilog="commands:\n" + COMMANDS,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("script", nargs="?", help="file with commands (default: read them from stdin)")
    parser.add_argument("--image", required=True, help="merged flash image (make qemu-image)")
    parser.add_argument("--qemu", default=os.environ.get("QEMU", "qemu-system-xtensa"), help="QEMU binary")
    parser.add_argument("--log", help="write the serial output to this file (- for stderr)")
    parser.add_argument("--boot-timeout", type=float, default=60, help="seconds to wait for Keira to boot")
    args = parser.parse_args()

    log = sys.stderr if args.log == "-" else open(args.log, "w") if args.log else None
    source = open(args.script) if args.script else sys.stdin
    interactive = source.isatty()
    try:
        with KeiraEmu(args.image, args.qemu, log, args.boot_timeout) as emu:
            if interactive:
                print("Keira is up, type commands (Ctrl-D to quit):\n" + COMMANDS, file=sys.stderr)
            for number, line in enumerate(source, 1):
                try:
                    if not run_command(emu, line):
                        break
                except EmuError as e:
                    if not interactive:
                        raise EmuError(f"{args.script or 'stdin'}:{number}: {e}") from None
                    print(f"error: {e}", file=sys.stderr)
                    continue
                if interactive:
                    print("ok", file=sys.stderr)
    except EmuError as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
