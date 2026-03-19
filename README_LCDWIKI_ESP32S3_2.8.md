# LCDWiki ESP32-S3 2.8 Bring-Up Guide

This document describes the custom board port for **LCDWiki/Hosyond 2.8-inch ESP32-S3 display board** in this repository.

## Scope

- Firmware base: `78/xiaozhi-esp32`
- Board target id: `lcdwiki-esp32-s3-2.8`
- Status: integrated, building, and flashing verified on real hardware

## Hardware Profile

- MCU: ESP32-S3
- Flash: 16MB
- PSRAM: 8MB
- LCD: ILI9341, SPI, 240x320
- Touch: FT5x06-compatible, I2C
- Audio codec: ES8311
- Amplifier control: GPIO1 (active-low enable)

## Pin Mapping Used

### Display (SPI)
- `SCK`: GPIO12
- `MOSI`: GPIO11
- `MISO`: GPIO13
- `CS`: GPIO10
- `DC`: GPIO46
- `BL`: GPIO45

### Touch (I2C)
- `SCL`: GPIO15
- `SDA`: GPIO16
- `RST`: GPIO18
- `INT`: GPIO17

### Audio (I2S + codec control)
- `MCLK`: GPIO4
- `BCLK`: GPIO5
- `WS/LRCK`: GPIO7
- `DOUT`: GPIO8
- `DIN`: GPIO6
- `Codec I2C SCL`: GPIO15
- `Codec I2C SDA`: GPIO16
- `PA EN`: GPIO1 (active-low)

### Other
- `Boot Button`: GPIO0
- `RGB LED`: GPIO42
- `UART0 TX/RX`: GPIO44 / GPIO43

## What Was Added/Changed

- New board directory:
  - `main/boards/lcdwiki-esp32-s3-2.8/`
- Kconfig/CMake integration for board selection.
- Board implementation with:
  - SPI LCD init
  - FT5x06 touch init
  - ES8311 audio codec init
  - BOOT button action
- Display orientation fix (horizontal mirror corrected).
- English wake-word model selection for this target (`HI ESP`).
- Status clock fallback to uptime format when RTC/network time is not available.

## Build Prerequisites (Windows)

1. Install ESP-IDF (5.5.x recommended for this repo).
2. Open `ESP-IDF PowerShell`.
3. In terminal:

```powershell
cmd /c "call C:\Users\Administrator\esp\esp-idf\export.bat"
```

## Build

From repo root:

```powershell
python scripts\release.py lcdwiki-esp32-s3-2.8
```

Expected outputs:

- `build\xiaozhi.bin`
- `build\merged-binary.bin`
- `releases\v2.2.4_lcdwiki-esp32-s3-2.8.zip` (version string may differ later)

## Flash

### ESP-IDF (recommended)

```powershell
cmd /c "call C:\Users\Administrator\esp\esp-idf\export.bat && idf.py -p COM5 flash"
```

Replace `COM5` with your board COM port.

### Direct esptool (optional)

```powershell
python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0xd000 build\ota_data_initial.bin 0x20000 build\xiaozhi.bin 0x800000 build\generated_assets.bin
```

## Run/Validation Checklist

1. Display boots correctly (no mirror issue).
2. Touch coordinates align with UI.
3. BOOT button enters listening mode.
4. Wake word test: say **"Hi ESP"**.
5. Audio test:
   - Mic capture triggers listening/wake behavior.
   - Speaker outputs response audio.
6. Clock area updates (uses uptime fallback if RTC time not set).

## Troubleshooting

### `idf.py` not recognized
Run through ESP-IDF shell or call `export.bat` before build/flash.

### Serial port busy
Close serial monitors/terminal tools using that COM port, unplug/replug board, then retry flash.

### No wake trigger
- Confirm wake phrase is **"Hi ESP"** for this build.
- Confirm board has a microphone path enabled.
- Test with BOOT button manual listening to separate wake-engine vs mic-path issues.

### No speaker output
- Verify amplifier wiring and GPIO1 active-low behavior.
- Confirm COM logs show codec initialized.

## VS Code Workflow

Use **Espressif IDF** extension (not PlatformIO as primary for this repo):

1. Install extension and configure ESP-IDF 5.5.x.
2. Open this repository folder.
3. Run build/release command in integrated terminal.
4. Flash with `idf.py -p COMx flash`.

## PlatformIO Note

This repository is maintained as ESP-IDF/CMake first.  
PlatformIO can be used for convenience tools, but official compatibility path is ESP-IDF.

