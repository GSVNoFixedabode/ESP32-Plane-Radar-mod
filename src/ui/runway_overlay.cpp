#include "ui/runway_overlay.h"

#include <lgfx/v1/lgfx_fonts.hpp>

#include <cmath>
#include <cstdlib>

#include "data/large_airports.h"
#include "hardware/display_font.h"
#include "services/radar_location.h"
#include "ui/radar_range.h"
#include "ui/radar_theme.h"

namespace ui::runway {
namespace {

constexpr float kKmPerDeg = 111.0f;
constexpr float kDegToRad = 3.14159265f / 180.0f;
constexpr size_t kMaxAirportLabels = 32;
constexpr size_t kBitWords = (data::large_airports::kAirportCount + 31) / 32;
uint32_t s_tested_bits[kBitWords];
uint32_t s_in_range_bits[kBitWords];
uint32_t s_label_pending_bits[kBitWords];

inline bool getBit(const uint32_t* bits, size_t idx) {
  return (bits[idx / 32] & (1UL << (idx % 32))) != 0;
}

inline void setBit(uint32_t* bits, size_t idx) {
  bits[idx / 32] |= (1UL << (idx % 32));
}

inline bool isAirportCategoryEnabled(uint8_t category) {
  switch (category) {
    case data::large_airports::kCatLarge:
      return radar::showRunwaysLarge();
    case data::large_airports::kCatMedium:
      return radar::showRunwaysMedium();
    case data::large_airports::kCatMilitary:
      return radar::showRunwaysMilitary();
    case data::large_airports::kCatSmall:
      return radar::showRunwaysSmall();
    default:
      return false;
  }
}

bool s_runway_label_ready = false;
bool s_runway_label_use_vlw = false;
float s_runway_label_vlw_size = 0.38f;
const lgfx::GFXfont* s_runway_label_gfx = &fonts::FreeSansBold12pt7b;

int measureVlwHeight(lgfx::LGFXBase& gfx, float size) {
  gfx.setTextSize(size);
  return gfx.fontHeight();
}

float findVlwSizeForHeight(lgfx::LGFXBase& gfx, int target_px) {
  float lo = 0.2f;
  float hi = 1.2f;
  for (int i = 0; i < 14; ++i) {
    const float mid = (lo + hi) * 0.5f;
    if (measureVlwHeight(gfx, mid) < target_px) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return hi;
}

void initRunwayLabelStyle(lgfx::LGFXBase& gfx) {
  if (s_runway_label_ready) {
    return;
  }

  const int target = radar::runwayLabelHeightPx();
  if (displayFontIsSmooth()) {
    s_runway_label_use_vlw = true;
    s_runway_label_vlw_size = findVlwSizeForHeight(gfx, target);
  } else {
    s_runway_label_gfx = &fonts::FreeSansBold12pt7b;
    s_runway_label_use_vlw = false;
  }
  s_runway_label_ready = true;
}

void applyRunwayLabelStyle(lgfx::LGFXBase& gfx) {
  if (s_runway_label_use_vlw) {
    displayFontSetSmoothSize(gfx, s_runway_label_vlw_size);
  } else {
    displayFontSetBitmap(gfx, s_runway_label_gfx);
  }
}

float e7ToDeg(int32_t e7) { return static_cast<float>(e7) * 1e-7f; }

void offsetKmFromCenter(float lat, float lon, float* dx_km, float* dy_km,
                        float* dist_km) {
  // Longitude degrees shrink toward the poles; scale by cos(latitude) so
  // east-west distance isn't overstated away from the equator.
  const float center_lat_rad =
      static_cast<float>(services::location::lat()) * kDegToRad;
  *dx_km = static_cast<float>(lon - services::location::lon()) * kKmPerDeg *
           cosf(center_lat_rad);
  *dy_km =
      static_cast<float>(lat - services::location::lat()) * kKmPerDeg;
  *dist_km = sqrtf((*dx_km) * (*dx_km) + (*dy_km) * (*dy_km));
}

void latLonToScreen(float lat, float lon, int* out_x, int* out_y) {
  const float outer_km = radar::rangeCurrent().outer_km;
  const float px_per_km =
      static_cast<float>(radar::gridOuterRadius()) / outer_km;

  float dx_km = 0.0f;
  float dy_km = 0.0f;
  float dist_km = 0.0f;
  offsetKmFromCenter(lat, lon, &dx_km, &dy_km, &dist_km);

  *out_x = radar::centerX() + static_cast<int>(lroundf(dx_km * px_per_km));
  *out_y = radar::centerY() - static_cast<int>(lroundf(dy_km * px_per_km));
}

int distSqFromCenter(int x, int y) {
  const int dx = x - radar::centerX();
  const int dy = y - radar::centerY();
  return dx * dx + dy * dy;
}

bool clipSegmentToDisc(float x0, float y0, float x1, float y1, float cx, float cy, float r,
                       float* ox0, float* oy0, float* ox1, float* oy1) {
  const float p0x = x0 - cx;
  const float p0y = y0 - cy;
  const float dx = x1 - x0;
  const float dy = y1 - y0;

  const float a = dx * dx + dy * dy;
  const float r_sq = r * r;

  if (a < 1e-4f) {
    if (p0x * p0x + p0y * p0y <= r_sq) {
      *ox0 = x0; *oy0 = y0;
      *ox1 = x1; *oy1 = y1;
      return true;
    }
    return false;
  }

  const float b = 2.0f * (p0x * dx + p0y * dy);
  const float c = (p0x * p0x + p0y * p0y) - r_sq;
  const float disc = b * b - 4.0f * a * c;

  if (disc < 0.0f) {
    return false;
  }

  const float sqrt_disc = sqrtf(disc);
  const float inv_2a = 0.5f / a;
  float t0 = (-b - sqrt_disc) * inv_2a;
  float t1 = (-b + sqrt_disc) * inv_2a;

  if (t0 > t1) {
    std::swap(t0, t1);
  }

  const float t_start = std::max(0.0f, t0);
  const float t_end = std::min(1.0f, t1);

  if (t_start > t_end) {
    return false;
  }

  *ox0 = x0 + t_start * dx;
  *oy0 = y0 + t_start * dy;
  *ox1 = x0 + t_end * dx;
  *oy1 = y0 + t_end * dy;
  return true;
}

void drawBoldRunwayLabel(lgfx::LGFXBase& gfx, const char* ident, int mx, int my) {
  const int tw = gfx.textWidth(ident);
  const int th = gfx.fontHeight();
  constexpr int kPadX = 2;
  constexpr int kPadY = 1;

  gfx.setTextDatum(textdatum_t::bottom_center);
  const int left = mx - tw / 2 - kPadX;
  const int top = my - th - kPadY;
  gfx.fillRect(left, top, tw + kPadX * 2, th + kPadY, radar::kColorBackground);
  gfx.setTextColor(radar::kColorRunwayLabel, radar::kColorBackground);
  gfx.drawString(ident, mx, my);
}

bool drawRunwayLine(lgfx::LGFXBase& gfx, const data::large_airports::Runway& rw, int y_offset) {
  const float le_lat = e7ToDeg(rw.le_lat_e7);
  const float le_lon = e7ToDeg(rw.le_lon_e7);
  const float he_lat = e7ToDeg(rw.he_lat_e7);
  const float he_lon = e7ToDeg(rw.he_lon_e7);

  int x0 = 0;
  int y0 = 0;
  int x1 = 0;
  int y1 = 0;
  latLonToScreen(le_lat, le_lon, &x0, &y0);
  latLonToScreen(he_lat, he_lon, &x1, &y1);

  const float cx = static_cast<float>(radar::centerX());
  const float cy = static_cast<float>(radar::centerY());
  const float r = static_cast<float>(radar::gridOuterRadius());

  float ox0 = 0.0f;
  float oy0 = 0.0f;
  float ox1 = 0.0f;
  float oy1 = 0.0f;

  if (!clipSegmentToDisc(static_cast<float>(x0), static_cast<float>(y0),
                        static_cast<float>(x1), static_cast<float>(y1),
                        cx, cy, r, &ox0, &oy0, &ox1, &oy1)) {
    return false;
  }

  gfx.drawWideLine(static_cast<int>(lroundf(ox0)), static_cast<int>(lroundf(oy0)) - y_offset,
                   static_cast<int>(lroundf(ox1)), static_cast<int>(lroundf(oy1)) - y_offset,
                   radar::runwayLineHalfWidth(), radar::kColorRunway);
  return true;
}

void offsetLabelFromCenter(int ax, int ay, int* lx, int* ly) {
  const int dx = ax - radar::centerX();
  const int dy = ay - radar::centerY();
  const float len = sqrtf(static_cast<float>(dx * dx + dy * dy));
  const int gap = radar::runwayLabelGapPx();
  if (len < 1.0f) {
    *lx = ax;
    *ly = ay - gap;
    return;
  }
  *lx = ax + static_cast<int>(lroundf(dx / len * static_cast<float>(gap)));
  *ly = ay + static_cast<int>(lroundf(dy / len * static_cast<float>(gap)));
}

void drawAirportLabel(lgfx::LGFXBase& gfx,
                      const data::large_airports::Airport& ap, int y_offset) {
  int ax = 0;
  int ay = 0;
  latLonToScreen(e7ToDeg(ap.lat_e7), e7ToDeg(ap.lon_e7), &ax, &ay);

  const int max_r = radar::gridOuterRadius();
  if (distSqFromCenter(ax, ay) > max_r * max_r) {
    return;
  }

  int lx = 0;
  int ly = 0;
  offsetLabelFromCenter(ax, ay, &lx, &ly);
  drawBoldRunwayLabel(gfx, ap.ident, lx, ly - y_offset);
}

}  // namespace

void drawLargeAirportRunways(lgfx::LGFXBase& gfx, int y_offset) {
  if (!radar::showRunways()) {
    return;
  }
  displayFontEnsureLoaded(gfx);
  const float radius_km = radar::rangeCurrent().outer_km * 1.15f;

  uint16_t label_airports[kMaxAirportLabels];
  size_t label_count = 0;

  memset(s_tested_bits, 0, sizeof(s_tested_bits));
  memset(s_in_range_bits, 0, sizeof(s_in_range_bits));
  memset(s_label_pending_bits, 0, sizeof(s_label_pending_bits));

  for (size_t i = 0; i < data::large_airports::kRunwayCount; ++i) {
    const auto& rw = data::large_airports::kRunways[i];
    const uint16_t ap_idx = rw.airport_idx;
    if (!getBit(s_tested_bits, ap_idx)) {
      setBit(s_tested_bits, ap_idx);
      const auto& ap = data::large_airports::kAirports[ap_idx];
      if (isAirportCategoryEnabled(ap.category)) {
        float dx_km = 0.0f;
        float dy_km = 0.0f;
        float dist_km = 0.0f;
        offsetKmFromCenter(e7ToDeg(ap.lat_e7), e7ToDeg(ap.lon_e7), &dx_km, &dy_km,
                           &dist_km);
        if (dist_km <= radius_km) {
          setBit(s_in_range_bits, ap_idx);
        }
      }
    }
    if (!getBit(s_in_range_bits, ap_idx)) {
      continue;
    }
    if (!drawRunwayLine(gfx, rw, y_offset)) {
      continue;
    }
    if (!getBit(s_label_pending_bits, ap_idx) && label_count < kMaxAirportLabels) {
      setBit(s_label_pending_bits, ap_idx);
      label_airports[label_count++] = ap_idx;
    }
  }

  if (label_count == 0) {
    return;
  }

  initRunwayLabelStyle(gfx);
  applyRunwayLabelStyle(gfx);
  for (size_t i = 0; i < label_count; ++i) {
    drawAirportLabel(gfx, data::large_airports::kAirports[label_airports[i]], y_offset);
  }
}

}  // namespace ui::runway
