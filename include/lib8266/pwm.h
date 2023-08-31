#ifndef LIB8266__PWM__H__
#define LIB8266__PWM__H__

#include "driver/pwm.h"

#include "lib8266/fixed.h"

esp_err_t ahoy_pwm_init(uint16_t cycle_us, uint8_t *pins, uint8_t num_pins);

esp_err_t ahoy_pwm_set(uint8_t num_channels, uint8_t *channels, ahoy_fixed_t *levels);

#endif
