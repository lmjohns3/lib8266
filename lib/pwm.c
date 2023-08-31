#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lib8266/base.h"
#include "lib8266/pwm.h"

static const char *TAG = "⚓ pwm";

static uint32_t pwm_cycle_us;
static uint32_t pwm_channel_to_pin[8];

esp_err_t ahoy_pwm_init(uint16_t cycle_us, uint8_t *pins, uint8_t num_pins) {
  if (pwm_cycle_us) return ESP_ERR_INVALID_ARG;
  pwm_cycle_us = cycle_us;
  num_pins = AHOY_MIN(num_pins, 8);
  for (uint8_t i = 0; i < num_pins; ++i) {
    pwm_channel_to_pin[i] = pins[i];
    ESP_LOGI(TAG, "enabled pin %d as channel %d", pins[i], i);
  }
  {
    uint32_t *duties = calloc(num_pins, sizeof(uint32_t));
    ESP_ERROR_CHECK(pwm_init(pwm_cycle_us, duties, num_pins, pwm_channel_to_pin));
    free(duties);
  }
  {
    float *phases = calloc(num_pins, sizeof(float));
    ESP_ERROR_CHECK(pwm_set_phases(phases));
    free(phases);
  }
  return ESP_OK;
}

esp_err_t ahoy_pwm_set(uint8_t num_channels, uint8_t *channels, ahoy_fixed_t *levels) {
  num_channels = AHOY_MIN(num_channels, 8);
  for (uint8_t i = 0; i < num_channels; ++i) {
    ESP_LOGD(TAG, "setting channel %d (pin %d) to %f",
             channels[i],
             pwm_channel_to_pin[channels[i]],
             AHOY_FIXED_TO_FLOAT(levels[i]));
    ESP_ERROR_CHECK(pwm_set_duty(channels[i], AHOY_FIXED_CLAMP_01(levels[i]) * pwm_cycle_us / AHOY_FIXED_1));
  }
  return pwm_start();
}
