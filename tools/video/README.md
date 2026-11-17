# Frame-by-frame screen recording (video capture)

Custom-build support for recording Keira's screen over USB-Serial/JTAG, frame by frame, for release videos.

- `src/services/screenstream/` — `ScreenStreamService`: sends delta+RLE frames (`LKF1`) and accepts commands
  (`S1`/`S0` stream, `K` key frame, `B<n><0|1>` / `T<n> <ms>` buttons, `L1`/`L0` frame-by-frame mode, `F` next frame).
- `src/keira/recclock.*` — virtual clock: while recording, apps, Lua (`util.time`, `delta`), wallpapers and the status bar
  clock follow video time (1/30 s per frame), so every frame is evenly spaced.
- While recording, the status bar shows an emulated battery (87 %, 4.02 V).
- `lilka_ui_clock.patch` — patch for the Lilka library (`.pio/libdeps/<env>/Lilka`): menu animations and the input dialog
  cursor read time through a weak `lilka_ui_millis()`, which Keira overrides with the virtual clock.
  Apply after the library is installed: `patch -p1 -d .pio/libdeps/v2/Lilka < tools/video/lilka_ui_clock.patch`.
