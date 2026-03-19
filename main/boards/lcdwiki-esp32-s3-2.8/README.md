# LCDWiki ESP32-S3 2.8 (ES3C28P)

This board target supports the LCDWiki / Hosyond 2.8-inch ESP32-S3 display board.

## Hardware mapping

- LCD: ILI9341 SPI, 240x320
- Touch: FT6336/FT5x06-compatible I2C
- Audio codec: ES8311
- Amplifier enable: GPIO1 (active-low on this board)
- RGB LED: GPIO42

## Build (Release Package)

```bash
python scripts/release.py lcdwiki-esp32-s3-2.8
```

This generates a firmware zip under `releases/`, for example:

- `releases/v2.2.4_lcdwiki-esp32-s3-2.8.zip`

## Flash (ESP-IDF CLI, recommended)

1. Open an `ESP-IDF PowerShell` terminal (or run `export.bat` first).
2. Build:

```bash
python scripts/release.py lcdwiki-esp32-s3-2.8
```

3. Flash using ESP-IDF:

```bash
idf.py -p COMx flash monitor
```

Or use esptool directly:

```bash
python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0xd000 build\ota_data_initial.bin 0x20000 build\xiaozhi.bin 0x800000 build\generated_assets.bin
```

## Flash (VS Code)

Use the official ESP-IDF extension for VS Code:

1. Install extension: `Espressif IDF`.
2. Configure ESP-IDF 5.5.x tools.
3. Open this repo folder.
4. In VS Code terminal, run:

```bash
python scripts/release.py lcdwiki-esp32-s3-2.8
idf.py -p COMx flash monitor
```

## PlatformIO note

This upstream repo is ESP-IDF/CMake-based and not maintained as a PlatformIO project.  
PlatformIO can still be used for serial monitoring, but build/flash compatibility is best with ESP-IDF.

## Manual target selection

or

```bash
idf.py set-target esp32s3
idf.py menuconfig
# Xiaozhi Assistant -> Board Type -> LCDWiki ESP32-S3 2.8-inch
idf.py build
idf.py -p <COM_PORT> flash monitor
```
