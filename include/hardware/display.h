#pragma once

#include "config.h"
#include "hardware/lgfx_config.hpp"

extern LGFX tft;

void displayInit();
config::DisplayModel displayGetModel();
void displaySetModel(config::DisplayModel model);
void displaySetModelFromPortal(const char* val);

uint8_t displayGetDayBrightness();
uint8_t displayGetNightBrightness();
uint8_t displayGetActiveBrightness();
void displaySetDayBrightness(uint8_t pct);
void displaySetNightBrightness(uint8_t pct);
void displayApplyBrightness(bool is_night_mode);
void displayApplyBrightnessPercent(uint8_t pct);
void displaySaveBrightnessFromPortal(const char* day_val, const char* night_val);

