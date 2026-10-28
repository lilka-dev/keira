# Running Keira in QEMU

Keira can boot in [Espressif's QEMU](https://github.com/espressif/qemu) (`qemu-system-xtensa -M esp32s3`)
for automated UI tests: buttons are pressed and screenshots are taken over the emulated serial port.

## How it works

- Board `lilka_v2_qemu` (`boards/lilka_v2_qemu/`) is `lilka_v2` with `Serial` on UART0 instead of
  USB-Serial/JTAG, and with `UARTInputService` enabled.
- `UARTInputService` (`src/services/uartinput/uartinput.h`) takes commands on the serial port:
  `@press <button> [ms]`, `@hold <button>`, `@release [button]` and `@screenshot`, which renders the
  status bar and the top app the same way the screenshot service does and sends a PNG as base64 lines.
  These commands work the same on a real Lilka built with `-DKEIRA_UART_INPUT`.
- `keira_emu.py` starts QEMU and speaks that protocol. Use it as a Python library or as a CLI.

The screen comes from Keira's app canvases, not from the emulated display: QEMU doesn't emulate the
ST7789. Anything an app draws straight to `lilka::display` and not to its canvas won't be in screenshots.

## Usage

```sh
brew install libgcrypt glib pixman sdl2 libslirp   # macOS only, QEMU's runtime libraries
make qemu-install                   # Espressif QEMU -> build-qemu/ (or pass QEMU=... to use another one)
make qemu-build qemu-image          # firmware + data/spiffs/ -> build-lilka_v2_qemu-docker/flash_image.bin
make qemu-run
```

`qemu-run` reads commands from stdin (or from `SCRIPT=file`):

```
press down
press a
wait "Launcher" 10
screenshot launcher.png
```

Run `tools/emu/keira_emu.py --help` for the full command list.

## Tests

`tests/emu/` holds pytest tests. Each test gets a freshly booted `emu`, and its serial log and last screen go
to `test_output/emu/`:

```python
def test_open_menu(emu):
    emu.press("a")
    emu.screenshot("test_output/emu/menu.png")
```

```sh
pip install pytest
make qemu-test
```

CI runs them in the `emulator-tests` job of `.github/workflows/firmware.yml` and uploads `test_output/emu`.
