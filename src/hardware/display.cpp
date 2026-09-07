#include "hardware/display.h"

#include <Preferences.h>

#include "hardware/display_font.h"

namespace {
constexpr char kDisplayPrefsNamespace[] = "display";
constexpr char kDisplayModelKey[] = "model";
constexpr char kDisplayDayBrightKey[] = "day_bright";
constexpr char kDisplayNightBrightKey[] = "night_bright";

config::DisplayModel s_current_model = config::DisplayModel::GC9A01_240x240;
uint8_t s_day_brightness = 100;
uint8_t s_night_brightness = 25;
uint8_t s_active_brightness_pct = 100;
}  // namespace

LGFX tft;

config::DisplayModel displayGetModel() {
  Preferences prefs;
  if (!prefs.begin(kDisplayPrefsNamespace, true)) {
    return s_current_model;
  }
  const uint8_t raw = prefs.getUChar(kDisplayModelKey,
      static_cast<uint8_t>(config::DisplayModel::GC9A01_240x240));
  prefs.end();
  return (raw == static_cast<uint8_t>(config::DisplayModel::GC9B71_360x360))
             ? config::DisplayModel::GC9B71_360x360
             : config::DisplayModel::GC9A01_240x240;
}

void displaySetModel(config::DisplayModel model) {
  Preferences prefs;
  if (!prefs.begin(kDisplayPrefsNamespace, false)) {
    return;
  }
  prefs.putUChar(kDisplayModelKey, static_cast<uint8_t>(model));
  prefs.end();
}

void displaySetModelFromPortal(const char* val) {
  if (val == nullptr) {
    return;
  }
  const config::DisplayModel model = (val[0] == '1')
                                         ? config::DisplayModel::GC9B71_360x360
                                         : config::DisplayModel::GC9A01_240x240;
  displaySetModel(model);
}

uint8_t displayGetDayBrightness() {
  Preferences prefs;
  if (!prefs.begin(kDisplayPrefsNamespace, true)) {
    return s_day_brightness;
  }
  const uint8_t val = prefs.getUChar(kDisplayDayBrightKey, s_day_brightness);
  prefs.end();
  return (val < 5 || val > 100) ? 100 : val;
}

uint8_t displayGetNightBrightness() {
  Preferences prefs;
  if (!prefs.begin(kDisplayPrefsNamespace, true)) {
    return s_night_brightness;
  }
  const uint8_t val = prefs.getUChar(kDisplayNightBrightKey, s_night_brightness);
  prefs.end();
  return (val < 5 || val > 100) ? 25 : val;
}

uint8_t displayGetActiveBrightness() {
  return s_active_brightness_pct;
}

void displaySetDayBrightness(uint8_t pct) {
  if (pct < 5) pct = 5;
  if (pct > 100) pct = 100;
  s_day_brightness = pct;
  Preferences prefs;
  if (prefs.begin(kDisplayPrefsNamespace, false)) {
    prefs.putUChar(kDisplayDayBrightKey, pct);
    prefs.end();
  }
}

void displaySetNightBrightness(uint8_t pct) {
  if (pct < 5) pct = 5;
  if (pct > 100) pct = 100;
  s_night_brightness = pct;
  Preferences prefs;
  if (prefs.begin(kDisplayPrefsNamespace, false)) {
    prefs.putUChar(kDisplayNightBrightKey, pct);
    prefs.end();
  }
}

void displayApplyBrightnessPercent(uint8_t pct) {
  if (pct < 5) pct = 5;
  if (pct > 100) pct = 100;
  s_active_brightness_pct = pct;
  const uint8_t raw = static_cast<uint8_t>((static_cast<uint32_t>(pct) * 255 + 50) / 100);
  tft.setBrightness(raw);
}

void displayApplyBrightness(bool is_night_mode) {
  const uint8_t pct = is_night_mode ? displayGetNightBrightness() : displayGetDayBrightness();
  displayApplyBrightnessPercent(pct);
}

void displaySaveBrightnessFromPortal(const char* day_val, const char* night_val) {
  if (day_val != nullptr && day_val[0] != '\0') {
    const int v = atoi(day_val);
    if (v >= 5 && v <= 100) {
      displaySetDayBrightness(static_cast<uint8_t>(v));
    }
  }
  if (night_val != nullptr && night_val[0] != '\0') {
    const int v = atoi(night_val);
    if (v >= 5 && v <= 100) {
      displaySetNightBrightness(static_cast<uint8_t>(v));
    }
  }
}

void displayInit() {
  s_current_model = displayGetModel();
  s_day_brightness = displayGetDayBrightness();
  s_night_brightness = displayGetNightBrightness();
  tft.initForModel(s_current_model);
  tft.init();
  tft.setRotation(0);
  displayApplyBrightness(false);
  tft.setTextWrap(false);
  displayFontInit();
}

