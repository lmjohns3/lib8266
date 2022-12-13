#include "pirate/io.h"

esp_err_t ahoy_io_init(uint8_t adc_clock_div,
                       uint32_t input_pin_bit_mask,
                       uint32_t output_pin_bit_mask) {
  if (adc_clock_div > 0) {
    adc_config_t cfg;
    cfg.mode = ADC_READ_TOUT_MODE;
    cfg.clk_div = adc_clock_div;
    esp_err_t err = adc_init(&cfg);
    if (err != ESP_OK) return err;
  }

  if (input_pin_bit_mask) {
    gpio_config_t cfg;
    cfg.pin_bit_mask = input_pin_bit_mask;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&cfg);
    if err != ESP_OK) return err;
  }

  if (output_pin_bit_mask) {
    gpio_config_t cfg;
    cfg.pin_bit_mask = output_pin_bit_mask;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&cfg);
    if err != ESP_OK) return err;
  }

  return ESP_OK;
}

esp_err_t ahoy_io_read_adc_raw(uint16_t *value) {
  return adc_read_fast(value, 1);
}

esp_err_t ahoy_io_read_adc_to_fixed(ahoy_fixed_t *value) {
  uint16_t raw;
  esp_err_t err = adc_read_fast(&raw, 1);
  if (err != ESP_OK) return err;
  *value = AHOY_TO_FIXED(raw / 1023.0);
  return ESP_OK;
}

inline esp_err_t ahoy_io_write_bit(gpio_num_t pin, uint32_t level) {
  return gpio_set_level(pin, level);
}

inline int ahoy_io_read_bit(gpio_num_t pin) {
  return gpio_get_level(pin);
}
