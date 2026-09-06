#include "services/time_service.h"

#include <Arduino.h>
#include <algorithm>
#include <cmath>
#include <esp_sntp.h>

namespace services::time {

namespace {

bool s_sntp_initialized = false;
constexpr time_t kMinValidTimestamp = 1700000000;  // Nov 2023

}  // namespace

void init() {
  if (s_sntp_initialized) {
    return;
  }
  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
  s_sntp_initialized = true;
  Serial.println("time: SNTP client initialized");
}

bool isTimeSynced() {
  const time_t now = ::time(nullptr);
  return now >= kMinValidTimestamp;
}

time_t utcTime() {
  const time_t now = ::time(nullptr);
  return (now >= kMinValidTimestamp) ? now : 0;
}

bool isSunBelowHorizon(double lat, double lon) {
  const time_t now = ::time(nullptr);
  if (now < kMinValidTimestamp) {
    return false;
  }

  struct tm utc_tm;
  gmtime_r(&now, &utc_tm);

  const int day_of_year = utc_tm.tm_yday + 1;
  const double hour = static_cast<double>(utc_tm.tm_hour) +
                      static_cast<double>(utc_tm.tm_min) / 60.0 +
                      static_cast<double>(utc_tm.tm_sec) / 3600.0;

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kDegToRad = kPi / 180.0;
  constexpr double kRadToDeg = 180.0 / kPi;

  // Fractional year in radians
  const double gamma =
      (2.0 * kPi / 365.0) * (day_of_year - 1 + (hour - 12.0) / 24.0);

  // Equation of time in minutes
  const double eqtime =
      229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma) -
                0.014615 * cos(2.0 * gamma) - 0.040849 * sin(2.0 * gamma));

  // Solar declination angle in radians
  const double decl =
      0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma) -
      0.006758 * cos(2.0 * gamma) + 0.000907 * sin(2.0 * gamma) -
      0.002697 * cos(3.0 * gamma) + 0.00148 * sin(3.0 * gamma);

  // True solar time in minutes
  const double time_offset = eqtime + 4.0 * lon;
  double tst = hour * 60.0 + time_offset;
  while (tst >= 1440.0) tst -= 1440.0;
  while (tst < 0.0) tst += 1440.0;

  // Solar hour angle in degrees -> radians
  const double ha_deg = (tst / 4.0) - 180.0;
  const double ha_rad = ha_deg * kDegToRad;

  const double lat_rad = lat * kDegToRad;

  // Solar zenith angle
  const double cos_zenith =
      sin(lat_rad) * sin(decl) + cos(lat_rad) * cos(decl) * cos(ha_rad);
  const double zenith_rad = acos(std::max(-1.0, std::min(1.0, cos_zenith)));
  const double zenith_deg = zenith_rad * kRadToDeg;

  // Solar elevation angle: 90 - zenith
  const double elev_deg = 90.0 - zenith_deg;

  // Sun is below horizon / civil twilight when elevation < -0.833 deg
  return elev_deg < -0.833;
}

}  // namespace services::time

