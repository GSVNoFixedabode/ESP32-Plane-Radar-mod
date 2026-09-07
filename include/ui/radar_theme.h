#pragma once

#include <cstdint>

#include "hardware/display.h"

namespace ui::radar {

inline int size() { return tft.width() > 0 ? tft.width() : 240; }
inline int centerX() { return size() / 2; }
inline int centerY() { return size() / 2; }

/** Outermost grid ring (inside edge labels). */
inline int gridOuterRadius() { return (size() * 107) / 240; }

/** N: offset from top edge (top_center, negative = up). */
inline int cardinalNorthOffsetY() { return size() >= 360 ? -2 : -1; }
/** S: offset from bottom edge (bottom_center, positive = down). */
inline int cardinalSouthOffsetY() { return size() >= 360 ? 4 : 3; }

/** Gap between scale label right edge and outer ring on the east spoke (px). */
inline int scaleGapFromOuterRing() { return size() >= 360 ? 9 : 6; }

/** Target cap height (px) for N/S/E/W. */
inline int cardinalLabelHeightPx() { return size() >= 360 ? 20 : 14; }
/** Scale label is this many px shorter than cardinals. */
inline int scaleBelowCardinalPx() { return size() >= 360 ? 4 : 3; }

constexpr int kRingCount = 4;

/** Shared grid stroke: drawWideLine half-width; rings use the same px count. */
inline float gridStrokeHalfWidth() { return size() >= 360 ? 1.2f : 1.0f; }

inline int centerDotRadius() { return size() >= 360 ? 3 : 2; }

/** Filled aircraft symbol (nose triangle). */
inline int aircraftNoseLenPx() { return size() >= 360 ? 12 : 8; }
inline int aircraftTailLenPx() { return size() >= 360 ? 5 : 3; }
inline int aircraftTailHalfPx() { return size() >= 360 ? 6 : 4; }
/** Track vector: ground distance covered in this many seconds at current gs. */
constexpr float kAircraftTrackHorizonSec = 60.0f;
/** Minimum visible vector when gs > 0 (px). */
inline int aircraftSpeedLineMinPx() { return size() >= 360 ? 3 : 2; }
/** Track line length uses this outer_km, not the active range preset. */
constexpr float kAircraftTrackRefOuterKm = 13.3f;
/** Shorter than full 60 s horizon at ref scale; ×1.5 length boost applied. */
constexpr float kAircraftTrackLengthScale = 1.5f / 5.0f;
/** drawWideLine half-width for speed vectors. */
inline float aircraftTrackLineHalfWidth() { return size() >= 360 ? 1.2f : 1.0f; }

inline float runwayLineWidthPx() { return size() >= 360 ? 3.0f : 2.0f; }
inline float runwayLineHalfWidth() { return runwayLineWidthPx() * 0.5f; }
inline int runwayLabelHeightPx() { return cardinalLabelHeightPx(); }
inline int runwayLabelGapPx() { return size() >= 360 ? 4 : 3; }
/** Gap from triangle edge to tag block (px). */
inline int aircraftLabelGapPx() { return size() >= 360 ? 2 : 1; }
/** Keep symbol centroid inside outer ring by at least this inset (px). */
inline int aircraftInsideRingInsetPx() {
  return aircraftNoseLenPx() + aircraftTailHalfPx() + 1;
}

/** Beyond-ring traffic: bearing cues on screen rim (correct direction, fixed radius). */
inline int beyondRingDotRadiusPx() { return size() >= 360 ? 6 : 4; }
inline int beyondRingScreenMarginPx() { return size() >= 360 ? 3 : 2; }
/** Target cap height (px) for aircraft tags (bold, slightly above scale label). */
inline int aircraftTagLabelHeightPx() { return size() >= 360 ? 19 : 13; }


/** RGB565 palette targets (applied in initPalette). */
constexpr uint8_t kBgR = 4;
constexpr uint8_t kBgG = 10;
constexpr uint8_t kBgB = 28;
constexpr uint8_t kGridR = 16;
constexpr uint8_t kGridG = 100;
constexpr uint8_t kGridB = 32;
// Aircraft category palette targets:
// Military: Always Red
constexpr uint8_t kMilitaryR = 255;
constexpr uint8_t kMilitaryG = 0;
constexpr uint8_t kMilitaryB = 0;

// Commercial / Airliners / Jets: Bright Cyan
constexpr uint8_t kCommercialR = 0;
constexpr uint8_t kCommercialG = 220;
constexpr uint8_t kCommercialB = 255;

// General Aviation / Propellers: Bright Lime Green
constexpr uint8_t kGaR = 50;
constexpr uint8_t kGaG = 240;
constexpr uint8_t kGaB = 50;

// Helicopters / Rotorcraft: Gold / Amber
constexpr uint8_t kHeliR = 255;
constexpr uint8_t kHeliG = 190;
constexpr uint8_t kHeliB = 0;

constexpr uint8_t kTrackR = 255;
constexpr uint8_t kTrackG = 0;
constexpr uint8_t kTrackB = 255;
constexpr uint8_t kTagTypeR = 255;
constexpr uint8_t kTagTypeG = 200;
constexpr uint8_t kTagTypeB = 0;
constexpr uint8_t kTagAltR = 90;
constexpr uint8_t kTagAltG = 200;
constexpr uint8_t kTagAltB = 255;
constexpr uint8_t kRunwayR = 56;
constexpr uint8_t kRunwayG = 150;
constexpr uint8_t kRunwayB = 170;
/** Lighter teal for ICAO labels (vs runway lines). */
constexpr uint8_t kRunwayLabelR = 110;
constexpr uint8_t kRunwayLabelG = 210;
constexpr uint8_t kRunwayLabelB = 230;

extern uint16_t kColorBackground;
extern uint16_t kColorGrid;
extern uint16_t kColorLabel;
extern uint16_t kColorCenter;
extern uint16_t kColorMilitary;
extern uint16_t kColorCommercial;
extern uint16_t kColorGA;
extern uint16_t kColorHeli;
extern uint16_t kColorEmergency;
extern uint16_t kColorTrackVector;
extern uint16_t kColorTagType;
extern uint16_t kColorTagAltitude;
extern uint16_t kColorRunway;
extern uint16_t kColorRunwayLabel;

}  // namespace ui::radar
