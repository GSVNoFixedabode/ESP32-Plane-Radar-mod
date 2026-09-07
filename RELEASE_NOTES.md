# Plane Radar v2.2.0

### 🌟 What's New in v2.2.0

- 📺 **Display Hardware Selection in Web Portal**:
  - Switch between **GC9A01 (1.28" 240×240 Round)** and **GC9B71 / GC9B72 (2.1" 360×360 Round)** directly from the `/param` or `/wifi` setup portal.
  - Setting persists across restarts in NVS without recompiling.

- 💡 **Live Real-Time Brightness Controls**:
  - Independent **Day Brightness** and **Night Brightness** sliders (5%–100%) with persistent NVS storage.
  - **Instant Live Preview**: Adjusting sliders immediately dims or brightens the screen in real time so you can visually calibrate the brightness before hitting Save.
  - **Dual Dimming Architecture**:
    - **Physical PWM on GPIO 5**: Smooth hardware backlight dimming via LEDC PWM on the 2.1" display.
    - **Software Color Palette Scaling**: Dynamic RGB luminance scaling across the entire UI for 1.28" display modules with hardwired backlights.

- ⚡ **Web Portal Reliability & Navigation**:
  - Added dedicated **Reboot** button to the main configuration portal menu between **Update** and **Exit**.
  - Resolved parameter buffer truncation in WiFiManager.
  - Implemented single-flight sequential async request queue (`/api/brightness`) with active connection cleanup to prevent TCP socket exhaustion during fast slider interactions.

---

### 📦 Flashing & Upgrading

#### Option 1: Web Browser Flash (PC or Android Mobile)
1. Open [espressif.github.io/esptool-js](https://espressif.github.io/esptool-js/) or [ESP Web Tools](https://web.esphome.io/) in Google Chrome or Microsoft Edge.
2. Connect your ESP32-C3 Super Mini via USB (USB-C to USB-C / OTG cable works on Android phones too).
3. Select `plane-radar-v2.2.0.bin` at flash offset `0x0`.
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

