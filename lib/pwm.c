#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lib8266/base.h"
#include "lib8266/pwm.h"

static const char *TAG = "pwm";

static uint32_t pwm_cycle_us;
static uint32_t pwm_channel_to_pin[8];

esp_err_t ahoy_pwm_init(uint16_t cycle_us, uint8_t n, uint8_t *pins) {
  if (pwm_cycle_us) {
    return ESP_INVALID_ARG;
  }
  pwm_cycle_us = cycle_us;
  n = AHOY_MIN(n, 8);
  for (uint8_t i = 0; i < n; ++i) {
    pwm_channel_to_pin[i] = pins[i];
    ESP_LOGI(TAG, "Enabled pin %d as channel %d", pins[i], i);
  }
  {
    uint32_t *duties = calloc(n, sizeof(uint32_t));
    AHOY_RETURN_IF_NOT_OK(pwm_init(pwm_cycle_us, duties, n, pwm_channel_to_pin));
    free(duties);
  }
  {
    int16_t *phases = calloc(n, sizeof(int16_t));
    AHOY_RETURN_IF_NOT_OK(pwm_set_phases(phases));
    free(phases);
  }
  return ESP_OK;
}

esp_err_t ahoy_pwm_set(uint8_t num_channels, uint8_t *channels, ahoy_fixed_t *levels) {
  num_channels = AHOY_MIN(num_channels, 8);
  for (uint8_t i = 0; i < num_channels; ++i) {
    ESP_LOGD(TAG, "Setting channel %d (pin %d) to %f",
             channel,
             pwm_channel_to_pin[channel],
             AHOY_FIXED_TO_FLOAT(level));
    AHOY_RETURN_IF_NOT_OK(
        pwm_set_duty(channels[i],
                     L8_FIXED_CLAMP_01(levels[i]) * pwm_cycle_us / L8_FIXED_1));
  }
  return pwm_start();
}
