# Xiaozhi ESP32 (jmlapido Custom Fork)

Custom fork of `78/xiaozhi-esp32` with real-board support for:

- **LCDWiki / Hosyond ESP32-S3 2.8-inch Display Board**
- Board target: `lcdwiki-esp32-s3-2.8`

## What Is Included

- New custom board integration in `main/boards/lcdwiki-esp32-s3-2.8`
- ILI9341 display + FT5x06 touch support
- ES8311 audio codec setup for this board
- Orientation fix (display/touch mirror corrected)
- English wake model enabled (`HI ESP`)
- Status clock fallback (uptime shown before RTC sync)
- Detailed board bring-up guide:
  - `README_LCDWIKI_ESP32S3_2.8.md`

## Hardware Mapping (Current)

### Display
- SPI: `SCK=12, MOSI=11, MISO=13, CS=10, DC=46, BL=45`

### Touch
- I2C: `SCL=15, SDA=16, RST=18, INT=17`

### Audio (ES8311)
- `MCLK=4, BCLK=5, WS=7, DOUT=8, DIN=6`
- AMP enable: `GPIO1` (active-low)

### Other
- Boot button: `GPIO0`
- RGB LED: `GPIO42`

## Build

Use ESP-IDF 5.5.x environment:

```powershell
cmd /c "call C:\Users\Administrator\esp\esp-idf\export.bat && python scripts\release.py lcdwiki-esp32-s3-2.8"
```

## Flash

```powershell
cmd /c "call C:\Users\Administrator\esp\esp-idf\export.bat && idf.py -p COM5 flash"
```

Replace `COM5` with your board COM port.

## Quick Validation

1. Display UI shows correctly (not mirrored).
2. Touch taps align to UI.
3. BOOT button enters listening mode.
4. Wake word test: say **"Hi ESP"**.
5. Verify mic input and speaker output.

## Upstream Base

- Upstream project: [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)

## License

This fork follows the upstream project license (MIT).

