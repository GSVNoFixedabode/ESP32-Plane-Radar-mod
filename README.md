# Plane Radar

<img width="800" height="450" alt="plane-radar" src="https://github.com/user-attachments/assets/716d0992-dab8-47ba-8f1a-2aec7f607419" />

**3D printed case (STL + assembly):** [MakerWorld](https://makerworld.com/en/models/2872376-esp32-plane-radar-live-ads-b-on-a-round-display#profileId-3207083) · **Firmware:** [Releases](https://github.com/MatixYo/ESP32-Plane-Radar/releases)

> [!NOTE]
> **Enhanced Edition (v2.1.0)**: This version includes several major features:
> - 📺 **Dual Display Hardware Support**: Native support for both **1.28" (240×240 GC9A01)** and **2.1" (360×360 GC9B71 / GC9B72)** round TFT displays, selectable directly in the web setup portal and persisted in NVS flash memory.
> - ⚡ **Flicker-Free 8-bit Double Buffering**: Smooth, tear-free SPI DMA double-buffered rendering adapting dynamically to 240×240 or 360×360 canvas sizes.
> - 💡 **LEDC PWM Backlight Control**: Hardware PWM backlight regulation on GPIO 5 (LEDC Channel 0).
> - 🚀 **Zero-Heap Stream Ingestion**: Direct socket streaming parser with on-the-fly field filtering in ArduinoJson, eliminating memory fragmentation and dropouts.
> - 🗺️ **Interactive OpenStreetMap Picker** & phone GPS / IP geolocation in the setup portal.
> - 🎨 **Aircraft Category Color Coding** (🔴 Military = Red, 🔵 Commercial = Cyan, 🟢 GA = Green, 🟡 Heli = Gold).
> - 🚨 **Flashing Emergency Aircraft** (real-time 400ms blink for squawk `7700`/`7600`/`7500` & active emergencies).
> - 🛫 **Multi-Category Airfield Overlays** (10,500+ global airfields with independent toggles for Major, Regional, Military, and GA).
> - 🌙 **Automatic & Manual Day / Night Mode** (automatic sunset-to-sunrise astronomical solar calculation via NTP + GPS coordinates, or manual dimmed ~45% brightness palette).
> - 📡 **Wireless OTA Updates** via PlatformIO (`supermini_ota`) and web browser (`/update`).
> - 🖥️ **Boot Status Display** (5-second screen showing IP and `plane-radar.local`).
>
> **Upgrading existing devices:** Devices running earlier partition tables must be flashed **once via USB** (using a PC or an **Android phone** with [esptool-js](https://espressif.github.io/esptool-js/)) with `plane-radar-v2.1.0-merged.bin` at offset `0x0` to write the dual OTA partition table. After this initial flash, all future updates can be done **100% wirelessly over Wi-Fi**.

Firmware for an **ESP32-C3 Super Mini** driving a round TFT display (**1.28″ GC9A01 240×240** or **2.1″ GC9B71/GC9B72 360×360**). Shows a circular **ADS-B radar** around your configured location, with **WiFiManager** for first-time setup.

## What it does

1. **Wi‑Fi setup** (if needed) — captive portal on AP **`PlaneRadar-Setup`**
2. **Radar** — live aircraft from [adsb.fi](https://opendata.adsb.fi/) on a sonar-style grid

After Wi‑Fi is saved, the device reconnects automatically; the radar runs in the main loop with periodic ADS-B updates (~5 s).

## Controls (BOOT, GPIO 9, active LOW)

| Action | Effect |
|--------|--------|
| **Short tap** | Cycle range preset (5 → 10 → 15 → 25 km); saved to flash |
| **Hold 3 s** | Clear Wi‑Fi, location, and units; reboot into setup portal |

During setup you can also hold BOOT at power-on to force a credential reset (same as the long press).

## Wi‑Fi setup portal

**First-time setup** (no saved Wi‑Fi):

1. Connect to **`PlaneRadar-Setup`**
2. Open **`http://plane-radar.local`** (preferred) or **`http://192.168.4.1`** — both are shown on the yellow setup screen; captive portal may open automatically
3. Set home Wi‑Fi, then save

**Reconfigure anytime** (after the device is on your network):

1. Open **`http://plane-radar.local`** or **`http://<device-ip>`** (e.g. from your router or serial log at boot)
2. Change Wi‑Fi, location, units, or runway overlays; save

The same portal runs on the setup AP and on the device’s LAN IP while connected to Wi‑Fi. mDNS hostname is `plane-radar` → **plane-radar.local** (`kPortalHostname` in `config.h`). Some clients resolve `.local` slowly; use the IP if needed.

**Custom fields** (stored in NVS):

| Field | Purpose |
|-------|---------|
| **📺 Display Hardware** | Choose between **GC9A01 (1.28" 240×240)** or **GC9B71 / GC9B72 (2.1" 360×360)** |
| **🗺️ Interactive Map** | Tap anywhere or drag the pin on OpenStreetMap to auto-fill latitude & longitude |
| **📍 Auto Locate** | One-tap phone GPS / IP geolocation fallback (`/api/geolocate`) |
| **Latitude / Longitude** | Radar center and ADS-B query position (defaults in `config.h` until set) |
| **Display distances in miles** | Ring scale label in **mi** instead of **km** (e.g. `6mi` vs `10km`) |
| **Auto Day/Night (Sunset to Sunrise)** | Automatically transitions between Day and Night palettes using astronomical solar calculations from GPS location and NTP time (no timezone configuration needed) |
| **Manual Night Mode (Dim palette)** | Software dimming mode (~45% brightness palette) override for comfortable nighttime viewing |
| **Major / International Airports** | Primary international hub runways (e.g. Heathrow `EGLL`, JFK `KJFK`, Schiphol `EHAM`) |
| **Regional / Medium Airports** | Regional & domestic airfields (e.g. Cambridge `EGSC`, Norwich `EGSH`, Biggin Hill `EGKB`) |
| **Military Airbases & Stations** | Military airbases, RAF & Air Force stations (e.g. RAF Coningsby `EGXC`, Lakenheath `EGUL`) |
| **Small / GA Airfields & Strips** | General aviation airfields, flying clubs, and light strips |

After boot and Wi‑Fi connection, the device displays a **5-second status screen** showing its IP address, mDNS address (`http://plane-radar.local`), and selected display driver. After a reset, the device reboots and shows the setup screen immediately.

## Radar display

### Grid

- Dark blue background, subdued green rings and crosshairs
- White **N / S / E / W** at the bezel; range label on the **east** spoke (ring 3 = ¾ of outer radius)
- White center dot
- **Automatic & Manual Day / Night Modes**: 
  - **Auto Day/Night**: Uses NOAA astronomical solar position calculation based on your device's exact GPS latitude & longitude and NTP UTC time. Automatically transitions to dimmed Night Mode when the sun dips below the horizon at dusk, and transitions back to Day Mode at dawn.
  - **Manual Night Mode**: Software dimming mode (~45% luminance across background, rings, aircraft, text, and runways) for bedside or dark-room viewing without requiring a physical backlight (`BL`) pin.
  - **Smooth Real-Time Transitions**: Mode switches occur dynamically in the background without needing a reboot or interrupting aircraft tracking.

Layout and colors: `include/ui/radar_theme.h`.

### Range presets

| Ring 3 label | Outer radius (aircraft scale) |
|------------|-------------------------------|
| 5 km / 3 mi | ~6.7 km |
| 10 km / 6 mi | ~13.3 km (default) |
| 15 km / 9 mi | ~20 km |
| 25 km / 16 mi | ~33.3 km |

Preset and miles/km choice persist across reboot (`planeradar` NVS namespace).

### Runways & Airfields

The radar includes a global database of **10,525 airports and 13,942 runways** categorized into 4 tiers:
- **Major / International Hubs** (1,094 airports): Heathrow, JFK, Schiphol, LAX, CDG, etc.
- **Regional / Medium Airports** (2,940 airports): Regional passenger, cargo & business airports.
- **Military Airbases & Stations** (521 bases): RAF stations, Air Force & Naval bases (e.g. RAF Coningsby `EGXC`, Lakenheath `EGUL`, Edwards AFB `KEDW`).
- **Small / GA Airfields** (5,970 airfields): General aviation flying clubs, paved & grass strips.

Features:
- Individual category switches in the Wi‑Fi setup portal (`/param` or `/wifi`).
- Teal runway geometry lines with 4-letter ICAO labels (e.g. `KJFK`).
- Filtered in $O(1)$ with a compact 1.3 KB bitset index in RAM.
- Re-generate or customize the dataset anytime: `python3 scripts/build_large_airports.py`.

### Aircraft & Color Coding

Aircraft are categorized and color-coded on the radar:

| Category | Indicator | Color |
| :--- | :--- | :--- |
| **Military** | Tactical callsigns, military airframes (`F35`, `EUFI`, `C17`, etc.), `adsb.fi` DB flags | 🔴 **Solid Red** |
| **Commercial Airliners / Jets** | Passenger jets, cargo, business jets (`A320`, `B738`, `B777`, `A350`, `GLF6`, etc.) | 🔵 **Bright Cyan** |
| **General Aviation / Props** | Light piston aircraft, trainers, turboprops, gliders (`C172`, `PA28`, `SR22`, `PC12`, etc.) | 🟢 **Bright Lime Green** |
| **Helicopters / Rotorcraft** | Rotorcraft and emergency medical / police helicopters (`H135`, `R44`, `AW139`, etc.) | 🟡 **Gold / Amber** |
| **In-Flight Emergency** | Active emergency or squawk `7700` (Emergency), `7600` (NORDO), `7500` (Hijack) | 🚨 **Flashing Red (400ms)** |

- **Inside the outer ring** — heading triangle, magenta speed vector (clipped at the ring), and category-colored callsign tag.
- **Outside the ring** (still within ADS-B fetch) — bearing dot on the screen rim in the aircraft's category color.
- **Tags** — placed toward the **center**: west (left) → tag on the **right** of the symbol; east (right) → tag on the **left**.

### ADS-B

- Source: `https://opendata.adsb.fi/api/v3/`
- Zero-heap streaming parser: streams response tokens straight from socket into ArduinoJson with dynamic memory filtering
- Fetch radius: `ui::radar::fetchRadiusKm()` — scales with the active preset to roughly the screen edge (so rim dots have data)
- Poll interval: `kAdsbFetchIntervalMs` (3 s) in `config.h`
- Ground aircraft hidden by default (`kAdsbShowGroundAircraft`)

## Configuration

Edit **`include/config.h`** for hardware and behavior:

| Area | Keys / notes |
|------|----------------|
| Portal | `kPortalApName`, `kPortalIp`, `kPortalHostname` / `kPortalHostUrl` (mDNS; needs `-DWM_MDNS` in `platformio.ini`) |
| Wi‑Fi timing | connect attempts, reconnect grace, portal timeout (`0` = no timeout) |
| BOOT | `kBootPin`, `kBootResetHoldMs`, `kBootTapMinMs` |
| Display SPI & PWM | `kDisplayPinRst`, `kDisplayPinCs`, `kDisplayPinDc`, `kDisplayPinMosi`, `kDisplayPinSclk`, `kDisplayPinBl`, `kDisplayPwmChannel`, `kDisplayPwmFreq` |
| Default location | `kDefaultRadarLat`, `kDefaultRadarLon` (until portal overrides) |
| ADS-B | `kAdsbFetchIntervalMs`, `kAdsbShowGroundAircraft` |

Range presets: `include/ui/radar_range.h` (`kRangePresets`).

## Project layout

```
include/
  config.h
  hardware/
    lgfx_config.hpp
    display.h
    display_font.h
  data/
    large_airports.h
  ui/
    radar_theme.h
    radar_range.h
    radar_display.h
    runway_overlay.h
    status_screens.h
  services/
    wifi_setup.h
    radar_location.h
    adsb_client.h
data/
  ui_font.vlw              — embedded smooth UI font (Noto Sans Bold)
scripts/
  build_large_airports.py
  merge_firmware.py
src/
  main.cpp
  data/
    large_airports_data.cpp
  hardware/
  ui/
  services/
```

## Wiring (Display ↔ ESP32-C3 Super Mini)

| Display Pin (GC9A01 / GC9B71) | ESP32-C3 Super Mini | Notes |
|-------------------------------|---------------------|-------|
| **VCC** | 3V3 / 5V | 3.3V logic (or 5V if module has onboard 3.3V LDO) |
| **GND** | GND | Ground |
| **RST** | GPIO **0** | Reset |
| **CS** | GPIO **1** | Chip Select |
| **DC** | GPIO **10** | Data / Command |
| **SDA (MOSI)** | GPIO **3** | SPI Data Out |
| **SCL (SCLK)** | GPIO **4** | SPI Clock |
| **BLK / BL / LED** | GPIO **5** | Backlight (LEDC PWM Channel 0) |
| **BOOT (Button)** | GPIO **9** | Onboard BOOT button on Super Mini |

## Build & Upload

### USB Serial Upload (Initial / cable flash)
```bash
pio run -t upload -e supermini
pio device monitor
```
- PlatformIO env: **`supermini`**
- Serial: **115200** baud
- USB CDC on boot enabled in `platformio.ini` for the Super Mini

### Wireless OTA Upload (Over Wi-Fi)
```bash
pio run -t upload -e supermini_ota --upload-port 192.168.0.xxx
```
- PlatformIO env: **`supermini_ota`**
- Or upload `firmware.bin` via the web browser at **`http://plane-radar.local/update`** or **`http://<device-ip>/update`**.

### Web-flashable release image (PC or Android Mobile)

Single `.bin` (`plane-radar-v2.1.0-merged.bin`) for [esptool-js](https://espressif.github.io/esptool-js/) and [ESP Web Tools](https://web.esphome.io/) (ESP32-C3, 4 MB, flash at **0x0**):

- **From a PC (Chrome / Edge):** Plug in the ESP32, visit [espressif.github.io/esptool-js](https://espressif.github.io/esptool-js/), select `plane-radar-v2.1.0-merged.bin` at `0x0`, and click Program.
- **From an Android Mobile Phone:** Plug the ESP32 into your phone using a USB-C to USB-C / OTG cable, open Chrome, navigate to [espressif.github.io/esptool-js](https://espressif.github.io/esptool-js/), and flash directly from your phone.

To build the merged binary locally:
```bash
pio run -e supermini
pio run -t merge -e supermini
```
*(Output: `.pio/build/supermini/firmware-merged.bin`).*

### CI and releases (GitHub Actions)

| Workflow | When | Output |
|----------|------|--------|
| [Build](.github/workflows/build.yml) | Push / PR to `main` | Artifact `plane-radar-supermini` (merged + split `.bin` files, ~90 days) |
| [Release](.github/workflows/release.yml) | Git tag `v*` (e.g. `v2.1.0`) | GitHub Release asset `plane-radar-v2.1.0.bin` + `.sha256` |

To ship a version users can download:

```bash
git tag v2.1.0
git push origin v2.1.0
```

The release workflow builds firmware in CI and attaches the merged image to the release. Download from **Releases** on GitHub, then flash at **0x0** (ESP32-C3, 4 MB).

## Dependencies

- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
