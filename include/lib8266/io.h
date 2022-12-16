#ifndef LIB8266__IO__H__
#define LIB8266__IO__H__

#include "lib8266/base.h"

#include "driver/adc.h"
#include "driver/gpio.h"

esp_err_t ahoy_io_init(uint8_t adc_clock_div,
                       uint32_t input_pin_bit_mask,
                       uint32_t output_pin_bit_mask);

#endif
