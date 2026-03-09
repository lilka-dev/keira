# Keira DynApp Demo

This is an example dynamically-loaded application (`.so`) for KeiraOS.

It demonstrates:
- Drawing shapes (circles, lines, rectangles, triangles)
- Animating objects (bouncing ball with trail, spinning hexagon)
- Reading controller input (D-pad, A/B buttons)
- Playing buzzer sounds
- Using the frame buffer (double-buffered rendering via `keira_queue_draw`)

## Building

### Prerequisites

You need the **Xtensa ESP32-S3 GCC toolchain**. If you have PlatformIO installed, it's already available at:

```
~/.platformio/packages/toolchain-xtensa-esp32s3/bin/
```

### Option A: Make

```bash
cd examples/dynapp_demo
make
```

This produces `demo.so`.

If your toolchain is in a different location:

```bash
make TOOLCHAIN_PATH=/path/to/xtensa-toolchain/bin
```

If the toolchain is already in your `PATH`:

```bash
make TOOLCHAIN_PATH=
```

#### Inspect

```bash
make info
```

Shows section layout, symbol table, and total size.

### Option B: CMake

```bash
cd examples/dynapp_demo
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../xtensa-esp32s3.cmake ..
make
```

This produces `build/demo.so`.

If your toolchain is in a custom location, set the `XTENSA_TOOLCHAIN_PATH` environment variable:

```bash
XTENSA_TOOLCHAIN_PATH=/path/to/xtensa-toolchain/bin \
    cmake -DCMAKE_TOOLCHAIN_FILE=../xtensa-esp32s3.cmake ..
make
```

The toolchain file (`xtensa-esp32s3.cmake`) searches in this order:
1. `XTENSA_TOOLCHAIN_PATH` environment variable
2. PlatformIO default path (`~/.platformio/packages/toolchain-xtensa-esp32s3/bin/`)
3. System `PATH`

## Running

1. Copy `demo.so` to the SD card on your Lilka device
2. Open the **File Manager** in KeiraOS
3. Navigate to and select `demo.so`
4. The app will start automatically

## Controls

| Button | Action |
|--------|--------|
| D-Pad  | Move the ball |
| A      | Speed boost |
| B      | Exit the app |

## Writing Your Own DynApp

1. Create a new `.c` file with `#include "keira_api.h"`
2. Implement `int app_main(int argc, char *argv[])` as the entry point
3. Use `keira_*` functions for display, controller, timing, and buzzer
4. Build with the same flags (see [Makefile](Makefile) or [CMakeLists.txt](CMakeLists.txt))
5. Copy the `.so` to the SD card and open it from the file manager

### Important Notes

- No standard library is available (`printf`, `malloc`, etc. are provided by Keira's symbol table)
- The app runs in the same address space as Keira — crashes will reset the device
- `argv[0]` contains the path to the `.so` file on the SD card
- Call `keira_queue_draw()` after finishing each frame to display it
- Return from `app_main` or press B to exit gracefully

### Available API

See [keira_api.h](keira_api.h) for the complete API reference with documentation.
