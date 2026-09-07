#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "config.h"

/** LovyanGFX device supporting both GC9A01 (240x240) and GC9B71/GC9B72 (360x360) */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Bus_SPI _bus;
  lgfx::Panel_GC9A01 _panel_gc9a01;
  lgfx::Panel_GC9B72 _panel_gc9b72;
  lgfx::Light_PWM _light;

public:
  void initForModel(config::DisplayModel model) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.freq_write = config::kDisplaySpiWriteHz;
      cfg.freq_read = 16000000;
      cfg.pin_sclk = static_cast<int>(config::kDisplayPinSclk);
      cfg.pin_mosi = static_cast<int>(config::kDisplayPinMosi);
      cfg.pin_miso = -1;
      cfg.pin_dc = static_cast<int>(config::kDisplayPinDc);
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      _bus.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = static_cast<int>(config::kDisplayPinBl);
      cfg.freq = config::kDisplayPwmFreq;
      cfg.pwm_channel = config::kDisplayPwmChannel;
      cfg.invert = false;
      _light.config(cfg);
    }
    if (model == config::DisplayModel::GC9B71_360x360) {
      _panel_gc9b72.setBus(&_bus);
      auto cfg = _panel_gc9b72.config();
      cfg.pin_cs = static_cast<int>(config::kDisplayPinCs);
      cfg.pin_rst = static_cast<int>(config::kDisplayPinRst);
      cfg.panel_width = 360;
      cfg.panel_height = 360;
      cfg.memory_width = 360;
      cfg.memory_height = 360;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.invert = false;
      cfg.rgb_order = false;
      _panel_gc9b72.config(cfg);
      _panel_gc9b72.setLight(&_light);
      setPanel(&_panel_gc9b72);
    } else {
      _panel_gc9a01.setBus(&_bus);
      auto cfg = _panel_gc9a01.config();
      cfg.pin_cs = static_cast<int>(config::kDisplayPinCs);
      cfg.pin_rst = static_cast<int>(config::kDisplayPinRst);
      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.memory_width = 240;
      cfg.memory_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.invert = config::kDisplayInvert;
      cfg.rgb_order = config::kDisplayRgbOrder;
      _panel_gc9a01.config(cfg);
      _panel_gc9a01.setLight(&_light);
      setPanel(&_panel_gc9a01);
    }
  }
};
