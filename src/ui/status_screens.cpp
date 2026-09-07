#include "ui/status_screens.h"

#include <lgfx/v1/lgfx_fonts.hpp>

#include <cmath>
#include <cstdio>
#include <cstddef>
#include <cstring>

#include "config.h"
#include "hardware/display.h"
#include "hardware/display_font.h"

namespace {

constexpr int kLineGap = 6;
inline int centerX() { return tft.width() > 0 ? tft.width() / 2 : 120; }
inline int centerY() { return tft.height() > 0 ? tft.height() / 2 : 120; }
inline int displayW() { return tft.width() > 0 ? tft.width() : 240; }
inline int displayH() { return tft.height() > 0 ? tft.height() : 240; }

constexpr int kSpinnerDotCount = 10;
constexpr float kSpinnerStepDeg = 6.0f;

struct SpinnerDot {
  int x = 0;
  int y = 0;
  bool drawn = false;
};

char s_connecting_ssid[33];
char s_ssid_line[33];
float s_spinner_angle_deg = -90.0f;
SpinnerDot s_spinner_dots[kSpinnerDotCount];
bool s_connecting_text_drawn = false;

constexpr auto& kGfxTitle = fonts::FreeSans18pt7b;
constexpr auto& kGfxBody = fonts::FreeSans12pt7b;
constexpr auto& kGfxDetail = fonts::Font2;
constexpr auto& kPortalGfxTitle = fonts::FreeSansBold18pt7b;
constexpr auto& kPortalGfxBody = fonts::FreeSansBold12pt7b;
constexpr auto& kPortalGfxEmphasis = fonts::FreeSansBold18pt7b;
constexpr auto& kConnectingGfxDetail = fonts::FreeSans9pt7b;

struct TextLine {
  const char* text;
  float vlw_size;
  const lgfx::GFXfont* gfx_font;
};

int lineHeightGfx(const lgfx::GFXfont* font) {
  displayFontSetBitmap(tft, font);
  return tft.fontHeight();
}

int lineHeightVlw(float size) {
  displayFontSetSmoothSize(tft, size);
  return tft.fontHeight();
}

void applyLineStyle(const TextLine& line) {
  if (displayFontIsSmooth()) {
    const float scale = (displayW() >= 360) ? 1.4f : 1.0f;
    displayFontSetSmoothSize(tft, line.vlw_size * scale);
  } else {
    displayFontSetBitmap(tft, line.gfx_font);
  }
}

void drawTextBlock(uint16_t bg, uint16_t fg, const TextLine* lines, size_t count) {
  tft.fillScreen(bg);
  tft.setTextColor(fg, bg);
  tft.setTextDatum(textdatum_t::middle_center);

  int total_h = 0;
  for (size_t i = 0; i < count; ++i) {
    if (displayFontIsSmooth()) {
      const float scale = (displayW() >= 360) ? 1.4f : 1.0f;
      total_h += lineHeightVlw(lines[i].vlw_size * scale);
    } else {
      total_h += lineHeightGfx(lines[i].gfx_font);
    }
    if (i + 1 < count) {
      total_h += (displayW() >= 360) ? (kLineGap + 3) : kLineGap;
    }
  }

  int y = (displayH() - total_h) / 2;
  const int gap = (displayW() >= 360) ? (kLineGap + 3) : kLineGap;
  for (size_t i = 0; i < count; ++i) {
    applyLineStyle(lines[i]);
    const int h =
        displayFontIsSmooth()
            ? lineHeightVlw(lines[i].vlw_size * ((displayW() >= 360) ? 1.4f : 1.0f))
            : lineHeightGfx(lines[i].gfx_font);
    tft.drawString(lines[i].text, centerX(), y + h / 2);
    y += h + gap;
  }
}

constexpr float kConnectingDetailVlw = 0.92f;

void applyConnectingDetailStyle() {
  if (displayFontIsSmooth()) {
    const float scale = (displayW() >= 360) ? 1.4f : 1.0f;
    displayFontSetSmoothSize(tft, kConnectingDetailVlw * scale);
  } else {
    displayFontSetBitmap(tft, &kConnectingGfxDetail);
  }
}

/** SSID on one line; truncate with … if wider than max text width. */
void fitSsidLine() {
  const int max_w = displayW() - 20;
  strncpy(s_ssid_line, s_connecting_ssid, sizeof(s_ssid_line) - 1);
  s_ssid_line[sizeof(s_ssid_line) - 1] = '\0';
  applyConnectingDetailStyle();
  if (tft.textWidth(s_ssid_line) <= max_w) {
    return;
  }
  const size_t len = strlen(s_connecting_ssid);
  for (size_t n = len; n > 0; --n) {
    snprintf(s_ssid_line, sizeof(s_ssid_line), "%.*s…", static_cast<int>(n),
             s_connecting_ssid);
    if (tft.textWidth(s_ssid_line) <= max_w) {
      return;
    }
  }
  strncpy(s_ssid_line, "…", sizeof(s_ssid_line) - 1);
  s_ssid_line[sizeof(s_ssid_line) - 1] = '\0';
}

void drawConnectingText() {
  tft.fillScreen(config::kColorBlack);

  tft.setTextDatum(textdatum_t::middle_center);
  tft.setTextColor(config::kTextOnBlack, config::kColorBlack);

  applyConnectingDetailStyle();
  const int max_w = displayW() - 20;
  const int detail_h = tft.fontHeight();
  const int gap = (displayW() >= 360) ? (kLineGap + 4) : kLineGap;
  const int total_h = detail_h * 2 + gap;
  const int block_top = (displayH() - total_h) / 2;
  constexpr int kPanelPadY = 8;
  tft.fillRect(centerX() - max_w / 2, block_top - kPanelPadY,
               max_w, total_h + kPanelPadY * 2, config::kColorBlack);

  int y = block_top;
  tft.drawString("Connecting to", centerX(), y + detail_h / 2);
  y += detail_h + gap;
  tft.drawString(s_ssid_line, centerX(), y + detail_h / 2);

  s_connecting_text_drawn = true;
}

void eraseSpinnerDots() {
  const int erase_r = (displayW() >= 360) ? 6 : 4;
  for (int i = 0; i < kSpinnerDotCount; ++i) {
    if (!s_spinner_dots[i].drawn) {
      continue;
    }
    tft.fillCircle(s_spinner_dots[i].x, s_spinner_dots[i].y, erase_r,
                   config::kColorBlack);
    s_spinner_dots[i].drawn = false;
  }
}

void drawSpinnerDots() {
  constexpr float kDegToRad = 0.01745329252f;
  const float head_rad = s_spinner_angle_deg * kDegToRad;
  const int spinner_r = (displayW() * 113) / 240;
  const int dot_r = (displayW() >= 360) ? 3 : 2;

  for (int i = 0; i < kSpinnerDotCount; ++i) {
    const float a = head_rad - static_cast<float>(i) * (6.283185307f / kSpinnerDotCount);
    const int x = centerX() + static_cast<int>(std::lround(std::cos(a) * spinner_r));
    const int y = centerY() + static_cast<int>(std::lround(std::sin(a) * spinner_r));

    const int fade = 255 - i * 22;
    const uint16_t color = tft.color565(0, fade, 0);
    tft.fillSmoothCircle(x, y, dot_r, color);

    s_spinner_dots[i].x = x;
    s_spinner_dots[i].y = y;
    s_spinner_dots[i].drawn = true;
  }
}

}  // namespace

void statusScreenConnectingBegin(const char* ssid) {
  const char* name = (ssid != nullptr && ssid[0] != '\0') ? ssid : "network";
  strncpy(s_connecting_ssid, name, sizeof(s_connecting_ssid) - 1);
  s_connecting_ssid[sizeof(s_connecting_ssid) - 1] = '\0';
  fitSsidLine();
  s_spinner_angle_deg = -90.0f;
  for (auto& dot : s_spinner_dots) {
    dot.drawn = false;
  }
  s_connecting_text_drawn = false;
  drawConnectingText();
  drawSpinnerDots();
}

void statusScreenConnectingTick() {
  if (!s_connecting_text_drawn) {
    drawConnectingText();
  }
  eraseSpinnerDots();
  s_spinner_angle_deg += kSpinnerStepDeg;
  if (s_spinner_angle_deg >= 270.0f) {
    s_spinner_angle_deg -= 360.0f;
  }
  drawSpinnerDots();
}

void statusScreenPortal() {
  const TextLine lines[] = {
      {"Wi-Fi setup", 1.15f, &kPortalGfxTitle},
      {"1. Join network:", 1.05f, &kPortalGfxBody},
      {config::kPortalApName, 1.12f, &kPortalGfxEmphasis},
      {"2. Open in browser:", 1.05f, &kPortalGfxBody},
      {config::kPortalHostUrl, 1.12f, &kPortalGfxEmphasis},
      {"or 192.168.4.1", 1.0f, &kPortalGfxBody},
  };
  drawTextBlock(config::kColorYellow, config::kTextOnYellow, lines,
                sizeof(lines) / sizeof(lines[0]));
}

void statusScreenConnectFailed() {
  const TextLine lines[] = {
      {"Could not connect", 1.15f, &kGfxTitle},
      {"Check Wi-Fi password", 1.0f, &kGfxBody},
      {"and signal strength.", 1.0f, &kGfxBody},
      {"Hold BOOT 3 sec", 1.0f, &kGfxBody},
      {"to reset Wi-Fi", 1.0f, &kGfxBody},
  };
  drawTextBlock(config::kColorYellow, config::kTextOnYellow, lines,
                sizeof(lines) / sizeof(lines[0]));
}

void statusScreenWifiReset() {
  const TextLine lines[] = {
      {"Wi-Fi reset", 1.15f, &kPortalGfxTitle},
      {"Restarting...", 1.05f, &kPortalGfxBody},
  };
  drawTextBlock(config::kColorYellow, config::kTextOnYellow, lines,
                sizeof(lines) / sizeof(lines[0]));
}

void statusScreenConnected(const char* ip, const char* hostname) {
  const TextLine lines[] = {
      {"Connected", 1.15f, &kPortalGfxTitle},
      {"IP address:", 1.0f, &kPortalGfxBody},
      {ip, 1.12f, &kPortalGfxEmphasis},
      {"Web config:", 1.0f, &kPortalGfxBody},
      {hostname, 1.10f, &kPortalGfxEmphasis},
  };
  drawTextBlock(config::kColorYellow, config::kTextOnYellow, lines,
                sizeof(lines) / sizeof(lines[0]));
}

namespace {
int s_last_update_percent = -1;
inline int barWidth() { return (displayW() * 170) / 240; }
inline int barHeight() { return (displayW() >= 360) ? 22 : 16; }
inline int barX() { return (displayW() - barWidth()) / 2; }
inline int barY() { return (displayH() * 110) / 240; }
constexpr int kBarRadius = 4;
}  // namespace

void statusScreenUpdateBegin(const char* title) {
  s_last_update_percent = -1;
  tft.fillScreen(config::kColorBlack);
  tft.setTextColor(config::kTextOnBlack, config::kColorBlack);
  tft.setTextDatum(textdatum_t::middle_center);

  const float scale = (displayW() >= 360) ? 1.4f : 1.0f;
  if (displayFontIsSmooth()) {
    displayFontSetSmoothSize(tft, 1.15f * scale);
  } else {
    displayFontSetBitmap(tft, &kPortalGfxTitle);
  }
  tft.drawString(title != nullptr ? title : "Firmware Update", centerX(), (displayH() * 48) / 240);

  if (displayFontIsSmooth()) {
    displayFontSetSmoothSize(tft, 0.95f * scale);
  } else {
    displayFontSetBitmap(tft, &kConnectingGfxDetail);
  }
  tft.setTextColor(tft.color565(120, 200, 255), config::kColorBlack);
  tft.drawString("Receiving image...", centerX(), (displayH() * 76) / 240);

  // Outer progress bar border
  tft.drawRoundRect(barX(), barY(), barWidth(), barHeight(), kBarRadius, config::kTextOnBlack);
  tft.fillRect(barX() + 2, barY() + 2, barWidth() - 4, barHeight() - 4, config::kColorBlack);

  // Warning text
  tft.setTextColor(config::kColorYellow, config::kColorBlack);
  if (displayFontIsSmooth()) {
    displayFontSetSmoothSize(tft, 0.88f * scale);
  } else {
    displayFontSetBitmap(tft, &kConnectingGfxDetail);
  }
  tft.drawString("Do not unplug power", centerX(), (displayH() * 185) / 240);

  statusScreenUpdateProgress(0);
}

void statusScreenUpdateProgress(int percent) {
  percent = constrain(percent, 0, 100);
  if (percent == s_last_update_percent) {
    return;
  }
  s_last_update_percent = percent;

  // Fill inner bar
  const int inner_max_w = barWidth() - 4;
  const int fill_w = (inner_max_w * percent) / 100;
  if (fill_w > 0) {
    tft.fillRoundRect(barX() + 2, barY() + 2, fill_w, barHeight() - 4, 2, tft.color565(0, 220, 80));
  }
  if (inner_max_w - fill_w > 0) {
    tft.fillRect(barX() + 2 + fill_w, barY() + 2, inner_max_w - fill_w, barHeight() - 4, config::kColorBlack);
  }

  // Draw percentage text
  char pct_buf[16];
  snprintf(pct_buf, sizeof(pct_buf), "%d%%", percent);
  tft.setTextDatum(textdatum_t::middle_center);
  tft.setTextColor(config::kTextOnBlack, config::kColorBlack);
  const float scale = (displayW() >= 360) ? 1.4f : 1.0f;
  if (displayFontIsSmooth()) {
    displayFontSetSmoothSize(tft, 1.10f * scale);
  } else {
    displayFontSetBitmap(tft, &kPortalGfxBody);
  }
  const int pct_y = (displayH() * 150) / 240;
  tft.fillRect(centerX() - 50, pct_y - 12, 100, 24, config::kColorBlack);
  tft.drawString(pct_buf, centerX(), pct_y);
}

void statusScreenUpdateEnd() {
  const TextLine lines[] = {
      {"Update complete", 1.15f, &kPortalGfxTitle},
      {"Rebooting...", 1.05f, &kPortalGfxBody},
  };
  drawTextBlock(config::kColorBlack, tft.color565(0, 255, 100), lines,
                sizeof(lines) / sizeof(lines[0]));
}

void statusScreenUpdateError(const char* message) {
  const TextLine lines[] = {
      {"Update failed", 1.15f, &kGfxTitle},
      {message != nullptr ? message : "Error writing flash", 1.0f, &kGfxBody},
      {"Please reboot device", 0.95f, &kConnectingGfxDetail},
  };
  drawTextBlock(config::kColorYellow, config::kTextOnYellow, lines,
                sizeof(lines) / sizeof(lines[0]));
}
