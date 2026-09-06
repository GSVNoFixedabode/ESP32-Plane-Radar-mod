#include "ui/radar_range.h"

#include "services/radar_location.h"
#include "services/time_service.h"
#include "ui/radar_theme.h"

#include <Preferences.h>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace ui::radar {

namespace {

constexpr char kPrefsNamespace[] = "planeradar";
constexpr char kPrefsRangeKey[] = "rangeIdx";
constexpr char kPrefsMilesKey[] = "useMiles";
constexpr char kPrefsNightModeKey[] = "nightMode";
constexpr char kPrefsAutoNightKey[] = "autoNight";
constexpr char kPrefsRunwaysKey[] = "showRwys";
constexpr char kPrefsRunwaysLargeKey[] = "showRwysL";
constexpr char kPrefsRunwaysMediumKey[] = "showRwysM";
constexpr char kPrefsRunwaysMilKey[] = "showRwysMil";
constexpr char kPrefsRunwaysSmallKey[] = "showRwysS";
constexpr uint8_t kDefaultRangeIndex = 1;  // 10 km ring
constexpr float kKmPerMile = 1.609344f;

Preferences s_prefs;
uint8_t s_range_index = kDefaultRangeIndex;
bool s_use_miles = false;
bool s_night_mode = false;
bool s_auto_night_mode = false;
bool s_show_runways = true;
bool s_show_runways_large = true;
bool s_show_runways_medium = true;
bool s_show_runways_mil = true;
bool s_show_runways_small = false;

void saveRangeIndex() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putUChar(kPrefsRangeKey, s_range_index);
  s_prefs.end();
}

void saveUseMiles() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsMilesKey, s_use_miles);
  s_prefs.end();
}

void saveNightMode() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsNightModeKey, s_night_mode);
  s_prefs.end();
}

void saveAutoNightMode() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsAutoNightKey, s_auto_night_mode);
  s_prefs.end();
}

void saveShowRunways() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsRunwaysKey, s_show_runways);
  s_prefs.putBool(kPrefsRunwaysLargeKey, s_show_runways_large);
  s_prefs.putBool(kPrefsRunwaysMediumKey, s_show_runways_medium);
  s_prefs.putBool(kPrefsRunwaysMilKey, s_show_runways_mil);
  s_prefs.putBool(kPrefsRunwaysSmallKey, s_show_runways_small);
  s_prefs.end();
}

bool portalCheckboxChecked(const char* value) {
  if (value == nullptr || value[0] == '\0') {
    return false;
  }
  // WiFiManager checkbox submits its value= attribute ("T", or "F" if we prefilled F).
  if ((value[0] == 'T' || value[0] == 't' || value[0] == 'F' || value[0] == 'f') &&
      value[1] == '\0') {
    return true;
  }
  return strcmp(value, "on") == 0;
}

}  // namespace

void rangeInit() {
  if (!s_prefs.begin(kPrefsNamespace, true)) {
    return;
  }
  const uint8_t saved = s_prefs.getUChar(kPrefsRangeKey, kDefaultRangeIndex);
  s_range_index =
      (saved < kRangePresetCount) ? saved : kDefaultRangeIndex;
  s_use_miles = s_prefs.getBool(kPrefsMilesKey, false);
  s_night_mode = s_prefs.getBool(kPrefsNightModeKey, false);
  s_auto_night_mode = s_prefs.getBool(kPrefsAutoNightKey, false);
  s_show_runways = s_prefs.getBool(kPrefsRunwaysKey, true);
  s_show_runways_large = s_prefs.getBool(kPrefsRunwaysLargeKey, true);
  s_show_runways_medium = s_prefs.getBool(kPrefsRunwaysMediumKey, true);
  s_show_runways_mil = s_prefs.getBool(kPrefsRunwaysMilKey, true);
  s_show_runways_small = s_prefs.getBool(kPrefsRunwaysSmallKey, false);
  s_prefs.end();
}

void rangeNext() {
  s_range_index = static_cast<uint8_t>((s_range_index + 1) % kRangePresetCount);
  saveRangeIndex();
}

const RangePreset& rangeCurrent() { return kRangePresets[s_range_index]; }

uint8_t rangeIndex() { return s_range_index; }

float fetchRadiusKm() {
  const float outer_km = rangeCurrent().outer_km;
  const float screen_r_px =
      static_cast<float>(kCenterX - kBeyondRingScreenMarginPx);
  return outer_km * (screen_r_px / static_cast<float>(kGridOuterRadius));
}

bool useMiles() { return s_use_miles; }

bool nightMode() { return s_night_mode; }

bool autoNightMode() { return s_auto_night_mode; }

bool effectiveNightMode() {
  if (s_auto_night_mode && services::time::isTimeSynced()) {
    return services::time::isSunBelowHorizon(services::location::lat(),
                                             services::location::lon());
  }
  return s_night_mode;
}

bool showRunways() {
  return s_show_runways && (s_show_runways_large || s_show_runways_medium ||
                            s_show_runways_mil || s_show_runways_small);
}

bool showRunwaysLarge() { return s_show_runways && s_show_runways_large; }

bool showRunwaysMedium() { return s_show_runways && s_show_runways_medium; }

bool showRunwaysMilitary() { return s_show_runways && s_show_runways_mil; }

bool showRunwaysSmall() { return s_show_runways && s_show_runways_small; }

void saveMilesFromPortal(const char* checkbox_value) {
  s_use_miles = portalCheckboxChecked(checkbox_value);
  saveUseMiles();
  Serial.printf("Distance units: %s\n", s_use_miles ? "miles" : "km");
}

void saveNightModeFromPortal(const char* checkbox_value) {
  s_night_mode = portalCheckboxChecked(checkbox_value);
  saveNightMode();
  Serial.printf("Night mode: %s\n", s_night_mode ? "on" : "off");
}

void saveAutoNightModeFromPortal(const char* checkbox_value) {
  s_auto_night_mode = portalCheckboxChecked(checkbox_value);
  saveAutoNightMode();
  Serial.printf("Auto night mode: %s\n", s_auto_night_mode ? "on" : "off");
}

void saveRunwaysFromPortal(const char* checkbox_value) {
  s_show_runways = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
  Serial.printf("Runway overlay: %s\n", s_show_runways ? "on" : "off");
}

void saveRunwaysLargeFromPortal(const char* checkbox_value) {
  s_show_runways_large = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
}

void saveRunwaysMediumFromPortal(const char* checkbox_value) {
  s_show_runways_medium = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
}

void saveRunwaysMilitaryFromPortal(const char* checkbox_value) {
  s_show_runways_mil = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
}

void saveRunwaysSmallFromPortal(const char* checkbox_value) {
  s_show_runways_small = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
}

void formatRing3Label(char* buf, size_t len, float ring3_km, bool use_miles) {
  if (use_miles) {
    const int mi = static_cast<int>(lroundf(ring3_km / kKmPerMile));
    snprintf(buf, len, "%dmi", mi);
  } else {
    const int km = static_cast<int>(lroundf(ring3_km));
    snprintf(buf, len, "%dkm", km);
  }
}

void formatCurrentRing3Label(char* buf, size_t len) {
  formatRing3Label(buf, len, rangeCurrent().ring3_km, s_use_miles);
}

void unitsReset() {
  s_use_miles = false;
  s_night_mode = false;
  s_auto_night_mode = false;
  s_show_runways = true;
  s_show_runways_large = true;
  s_show_runways_medium = true;
  s_show_runways_mil = true;
  s_show_runways_small = false;
  if (s_prefs.begin(kPrefsNamespace, false)) {
    s_prefs.remove(kPrefsMilesKey);
    s_prefs.remove(kPrefsNightModeKey);
    s_prefs.remove(kPrefsAutoNightKey);
    s_prefs.remove(kPrefsRunwaysKey);
    s_prefs.remove(kPrefsRunwaysLargeKey);
    s_prefs.remove(kPrefsRunwaysMediumKey);
    s_prefs.remove(kPrefsRunwaysMilKey);
    s_prefs.remove(kPrefsRunwaysSmallKey);
    s_prefs.end();
  }
}

}  // namespace ui::radar
