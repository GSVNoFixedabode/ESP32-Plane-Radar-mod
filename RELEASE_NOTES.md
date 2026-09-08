# Plane Radar v2.2.1

### 🌟 What's New in v2.2.1

- 🖥️ **Band Double-Buffered Rendering (Zero-Blink, Zero Heap Bloat)**:
  - Replaced full-frame sprite allocation with a static **43.2 KB Band Sprite** double-buffering architecture on 360×360 displays (and 57.6 KB on 240×240).
  - Renders slices directly off-screen and blits to display in 20 ms without wiping the panel to black (`fillScreen`).
  - Leaves **>145 KB of free heap** permanently available for TLS encryption handshakes and ESP32 hardware SHA acceleration (`esp-sha`), completely eliminating `HTTP -1` / SSL memory errors.

- 📐 **Exact Geometric Runway Clipping & Artifact Removal**:
  - Replaced integer-based line-circle intersection arithmetic with exact floating-point quadratic clipping (`clipSegmentToDisc`), fixing 32-bit integer overflow on runways $>20\text{ km}$ away.
  - Runways and airport labels are now strictly filtered to the visible radar radius (`outer_km * 1.15`), eliminating phantom runway snippets and perimeter edge clutter.

- 🎨 **RGB/BGR Color Ordering & Contrast Fixes**:
  - Unified color mapping through `makeColor()` across all display elements.
  - Fixes aircraft tag type (Yellow), altitude (Cyan), runway lines (Teal), and grid colors on BGR panel displays.

- 🏷️ **Aircraft Tag Collision De-Confliction**:
  - Added real-time bounding-box collision detection for aircraft tags.
  - Automatically shifts overlapping tags vertically when multiple aircraft fly in close proximity, ensuring full readability of callsigns, aircraft types, and altitudes.

- 📶 **WiFi & Network Resilience**:
  - Restored full `19.5 dBm` RF transmit power for onboard ceramic antenna signal strength.
  - Extended TLS connect/handshake timeouts to 8000 ms to handle network latency gracefully.

- ⚡ **Web Portal & Navigation Polish**:
  - Dedicated **Reboot** button on main configuration portal menu between **Update** and **Exit**.
  - Live real-time brightness calibration sliders with instant screen feedback for both Day and Night modes.
  - Display hardware selection (GC9A01 240×240 vs GC9B71 360×360) in web portal with persistent NVS storage.

---

### 📦 Flashing & Upgrading

#### Option 1: Web Browser Flash (PC or Android Mobile)
1. Open [espressif.github.io/esptool-js](https://espressif.github.io/esptool-js/) or [ESP Web Tools](https://web.esphome.io/) in Google Chrome or Microsoft Edge.
2. Connect your ESP32-C3 Super Mini via USB (USB-C to USB-C / OTG cable works on Android phones too).
3. Select `plane-radar-v2.2.1.bin` at flash offset `0x0`.
4. Click **Program**.

#### Option 2: Over-The-Air (OTA) Wireless Update
1. Open `http://plane-radar.local/update` (or `http://<device-ip>/update`) in your browser.
2. Select the firmware file and click **Update**.

#### Option 3: PlatformIO
```bash
# Via USB Serial
pio run -e supermini -t upload

# Via OTA Wi-Fi
pio run -e supermini_ota -t upload --upload-port <device-ip>
```

