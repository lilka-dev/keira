# Boot menu and partition table

Keira for Lilka v2 is built with the boot menu and the partition table from
[lilka-bootloader](https://github.com/lilka-dev/lilka-bootloader) instead of the stock Arduino ones
(see `platformio.ini`). They're the same on every board, so `KeiraOS_merged*.bin` from the releases
writes all of it at `0x0`, e.g. with [flasher.lilka.dev](https://flasher.lilka.dev/).

- `lilka_v2/bootloader.bin`: boot menu (hold START on reset): Keira OS, Retro-Go, firmware from the SD card
- `lilka_v2/partitions.csv`: Keira `app0`/`app1`, Retro-Go apps, SPIFFS

Retro-Go is installed from Keira: open `retro-go_lilka.img` from the SD card (or the catalog) in the
file manager.

Boards flashed with the stock layout keep it on OTA updates (only the app is updated); flash
`KeiraOS_merged*.bin` once over USB to switch.

## Updating

Built from lilka-bootloader `b070661`:

```bash
make bootloader   # in lilka-bootloader
cp bootloader/build-docker/bootloader/bootloader.bin <keira>/boot/lilka_v2/
cp partitions.csv <keira>/boot/lilka_v2/
```

Keep this file's commit reference in sync.
