#ifndef LIB8266__SHT3X__H__
#define LIB8266__SHT3X__H__

#include "lib8266/fixed.h"

#include "driver/i2c.h"

typedef enum {
  AHOY_SHT3X_HIGH = 0,
  AHOY_SHT3X_MEDIUM = 1,
  AHOY_SHT3X_LOW = 2,
} ahoy_sht3x_repeatability_t;

typedef struct {
  uint16_t sda_pin;
  uint16_t scl_pin;
  uint16_t clk_stretch_tick;
  uint16_t read_timeout_msec;
  bool read_with_clock_stretch;
  ahoy_sht3x_repeatability_t repeatability;
} ahoy_sht3x_t;

typedef ahoy_sht3x_t *ahoy_sht3x_handle_t;

esp_err_t ahoy_sht3x_init(const ahoy_sht3x_handle_t sht3x);

esp_err_t ahoy_sht3x_read_once(const ahoy_sht3x_handle_t sht3x,
                               ahoy_fixed_t *temperature_degc,
                               ahoy_fixed_t *relative_humidity);

#endif
