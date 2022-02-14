#ifndef PIRATE__PWM__H__
#define PIRATE__PWM__H__

#include "driver/pwm.h"

esp_err_t ahoy_pwm_init(uint8_t n, uint8_t *pins, uint16_t cycle_us);
esp_err_t ahoy_pwm_set(uint8_t num_channels, uint8_t *channels, ahoy_fixed_t *levels) {

#endif
