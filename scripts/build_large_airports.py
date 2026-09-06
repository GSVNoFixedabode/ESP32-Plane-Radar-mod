#!/usr/bin/env python3
"""Build runway dataset from OurAirports (Large, Medium, Military, Small)."""

from __future__ import annotations

import csv
import io
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_H = ROOT / "include" / "data" / "large_airports.h"
OUT_CPP = ROOT / "src" / "data" / "large_airports_data.cpp"

AIRPORTS_URL = (
    "https://raw.githubusercontent.com/davidmegginson/ourairports-data/main/"
    "airports.csv"
)
RUNWAYS_URL = (
    "https://raw.githubusercontent.com/davidmegginson/ourairports-data/main/"
    "runways.csv"
)

MIL_KEYWORDS = [
    "RAF ", "AFB", "AIR BASE", "NAVAL AIR", "NAS ", "ARMY AIR", "MCAS",
    "AIR FORCE BASE", "MILITARY", "BASE AERIENNE", "FLIEGERHORST",
    "LUFTWAFFEN", "AERODROME MILITAIRE", "AB ", "A.B.", "KASARNE", "AERODROMO MILITAR"
]


def fetch_csv(url: str) -> list[dict[str, str]]:
    with urllib.request.urlopen(url, timeout=60) as resp:
        text = resp.read().decode("utf-8")
    return list(csv.DictReader(io.StringIO(text)))


def coord_e7(s: str | None) -> int | None:
    if not s or not s.strip():
        return None
    return int(round(float(s) * 1e7))


def is_h_designator(s: str) -> bool:
    if not s or s[0] != "H":
        return False
    rest = s[1:]
    if not rest:
        return True
    if rest[0] in "-_":
        return True
    return rest.isdigit()


def is_helipad(row: dict[str, str]) -> bool:
    le = (row.get("le_ident") or "").strip().upper()
    he = (row.get("he_ident") or "").strip().upper()
    if not is_h_designator(le) and not is_h_designator(he):
        return False
    try:
        length_ft = int(row.get("length_ft") or 0)
    except ValueError:
        length_ft = 0
    if is_h_designator(le) and is_h_designator(he):
        return True
    return length_ft < 2500


def build_dataset() -> tuple[
    list[tuple[str, int, int, int]],
    list[tuple[int, int, int, int, int, int]],
]:
    print("Fetching OurAirports airports & runways...")
    airports = fetch_csv(AIRPORTS_URL)
    runways = fetch_csv(RUNWAYS_URL)

    ap_candidates: dict[str, tuple[int, int, int]] = {}
    for a in airports:
        ident = (a.get("ident") or "").strip()
        if len(ident) not in (3, 4):
            continue
        t = a.get("type", "")
        if t not in ("large_airport", "medium_airport", "small_airport"):
            continue
        lat = coord_e7(a.get("latitude_deg"))
        lon = coord_e7(a.get("longitude_deg"))
        if lat is None or lon is None:
            continue

        name = (a.get("name") or "").upper()
        is_mil = any(k in name for k in MIL_KEYWORDS)
        if is_mil:
            cat = 2  # Military
        elif t == "large_airport":
            cat = 0  # Large
        elif t == "medium_airport":
            cat = 1  # Medium
        elif t == "small_airport":
            cat = 3  # Small
        else:
            continue
        ap_candidates[ident] = (lat, lon, cat)

    airport_rows = sorted(
        (ident, lat, lon, cat)
        for ident, (lat, lon, cat) in ap_candidates.items()
    )
    temp_idx = {ident: idx for idx, (ident, _, _, _) in enumerate(airport_rows)}

    used_airports = set()
    segments: list[tuple[int, int, int, int, int, int]] = []
    for r in runways:
        if r.get("closed") == "1":
            continue
        ident = (r.get("airport_ident") or "").strip()
        if ident not in temp_idx:
            continue
        if is_helipad(r):
            continue
        try:
            length_ft = int(r.get("length_ft") or 0)
        except ValueError:
            continue
        if length_ft <= 0:
            continue
        le_lat = coord_e7(r.get("le_latitude_deg"))
        le_lon = coord_e7(r.get("le_longitude_deg"))
        he_lat = coord_e7(r.get("he_latitude_deg"))
        he_lon = coord_e7(r.get("he_longitude_deg"))
        if None in (le_lat, le_lon, he_lat, he_lon):
            continue
        length_m = int(round(length_ft * 0.3048))
        segments.append(
            (
                temp_idx[ident],
                le_lat,
                le_lon,
                he_lat,
                he_lon,
                length_m,
            )
        )
        used_airports.add(ident)

    final_airports = sorted(
        (ident, lat, lon, cat)
        for ident, lat, lon, cat in airport_rows
        if ident in used_airports
    )
    final_idx = {ident: idx for idx, (ident, _, _, _) in enumerate(final_airports)}

    final_segments: list[tuple[int, int, int, int, int, int]] = []
    for old_idx, le_lat, le_lon, he_lat, he_lon, length_m in segments:
        ident = airport_rows[old_idx][0]
        final_segments.append(
            (
                final_idx[ident],
                le_lat,
                le_lon,
                he_lat,
                he_lon,
                length_m,
            )
        )

    final_segments.sort(key=lambda row: (row[0], -row[5]))
    return final_airports, final_segments


def render_header(airport_count: int, segment_count: int) -> str:
    return "\n".join(
        [
            "// Generated by scripts/build_large_airports.py — do not edit.",
            "#pragma once",
            "",
            "#include <cstddef>",
            "#include <cstdint>",
            "",
            "namespace data::large_airports {",
            "",
            "enum AirportCategory : uint8_t {",
            "  kCatLarge = 0,",
            "  kCatMedium = 1,",
            "  kCatMilitary = 2,",
            "  kCatSmall = 3,",
            "};",
            "",
            "struct Airport {",
            "  char ident[5];",
            "  int32_t lat_e7;",
            "  int32_t lon_e7;",
            "  uint8_t category;",
            "};",
            "",
            "struct Runway {",
            "  uint16_t airport_idx;",
            "  int32_t le_lat_e7;",
            "  int32_t le_lon_e7;",
            "  int32_t he_lat_e7;",
            "  int32_t he_lon_e7;",
            "  uint16_t length_m;",
            "};",
            "",
            f"constexpr size_t kAirportCount = {airport_count};",
            f"constexpr size_t kRunwayCount = {segment_count};",
            "",
            "extern const Airport kAirports[];",
            "extern const Runway kRunways[];",
            "",
            "}  // namespace data::large_airports",
            "",
        ]
    )


def render_cpp(
    airport_rows: list[tuple[str, int, int, int]],
    segments: list[tuple[int, int, int, int, int, int]],
) -> str:
    lines = [
        "// Generated by scripts/build_large_airports.py — do not edit.",
        '#include "data/large_airports.h"',
        "",
        "namespace data::large_airports {",
        "",
        "const Airport kAirports[] = {",
    ]
    for ident, lat, lon, cat in airport_rows:
        lines.append(f'  {{"{ident}", {lat}, {lon}, {cat}}},')
    lines += [
        "};",
        "",
        "const Runway kRunways[] = {",
    ]
    for airport_idx, le_lat, le_lon, he_lat, he_lon, length_m in segments:
        lines.append(
            f"  {{{airport_idx}, {le_lat}, {le_lon}, {he_lat}, {he_lon}, {length_m}}},"
        )
    lines += [
        "};",
        "",
        "}  // namespace data::large_airports",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    airport_rows, segments = build_dataset()
    header = render_header(len(airport_rows), len(segments))
    cpp = render_cpp(airport_rows, segments)

    OUT_H.parent.mkdir(parents=True, exist_ok=True)
    OUT_CPP.parent.mkdir(parents=True, exist_ok=True)
    OUT_H.write_text(header, encoding="utf-8")
    OUT_CPP.write_text(cpp, encoding="utf-8")
    print(
        f"wrote {OUT_H.name} + {OUT_CPP.name} "
        f"({len(segments)} segments, {len(airport_rows)} airports)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
