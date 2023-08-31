#include "lib8266/io.h"

esp_err_t ahoy_io_init_adc(uint8_t clock_div) {
  adc_config_t cfg;
  cfg.mode = ADC_READ_TOUT_MODE;
  cfg.clk_div = clock_div;
  return adc_init(&cfg);
}

esp_err_t ahoy_io_init_inputs(uint32_t pin_bit_mask, bool enable_pullup) {
  gpio_config_t cfg;
  cfg.pin_bit_mask = pin_bit_mask;
  cfg.mode = GPIO_MODE_INPUT;
  cfg.pull_up_en = enable_pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
  cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
  cfg.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&cfg));
  for (uint8_t i = 0; i < GPIO_NUM_MAX; ++i) {
    if (pin_bit_mask & BIT(i)) gpio_get_level(i);
  }
  return ESP_OK;
}

esp_err_t ahoy_io_init_outputs(uint32_t pin_bit_mask) {
  gpio_config_t cfg;
  cfg.pin_bit_mask = pin_bit_mask;
  cfg.mode = GPIO_MODE_OUTPUT;
  cfg.pull_up_en = GPIO_PULLUP_DISABLE;
  cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
  cfg.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&cfg));
  for (uint8_t i = 0; i < GPIO_NUM_MAX; ++i) {
    if (pin_bit_mask & BIT(i)) {
      gpio_set_level(i, 1);
      gpio_set_level(i, 0);
    }
  }
  return ESP_OK;
}
