#ifndef PIRATE__IO__H__
#define PIRATE__IO__H__

#include "pirate/base.h"

#include "driver/adc.h"
#include "driver/gpio.h"

esp_err_t ahoy_io_init();
esp_err_t ahoy_io_read_adc_fast(ahoy_fixed_t &value);

#endif
