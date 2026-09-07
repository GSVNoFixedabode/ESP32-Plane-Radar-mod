#pragma once

#include "config.h"
#include "hardware/lgfx_config.hpp"

extern LGFX tft;

void displayInit();
config::DisplayModel displayGetModel();
void displaySetModel(config::DisplayModel model);
void displaySetModelFromPortal(const char* val);

