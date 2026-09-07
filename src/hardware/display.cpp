#include "hardware/display.h"

#include <Preferences.h>

#include "hardware/display_font.h"

namespace {
constexpr char kDisplayPrefsNamespace[] = "display";
constexpr char kDisplayModelKey[] = "model";

config::DisplayModel s_current_model = config::DisplayModel::GC9A01_240x240;
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

void displayInit() {
  s_current_model = displayGetModel();
  tft.initForModel(s_current_model);
  tft.init();
  tft.setRotation(0);
  tft.setBrightness(255);
  tft.setTextWrap(false);
  displayFontInit();
}

