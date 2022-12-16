#include "lib8266/io.h"

esp_err_t ahoy_io_init(uint8_t adc_clock_div,
                       uint32_t input_pin_bit_mask,
                       uint32_t output_pin_bit_mask) {
  if (adc_clock_div > 0) {
    adc_config_t cfg;
    cfg.mode = ADC_READ_TOUT_MODE;
    cfg.clk_div = adc_clock_div;
    AHOY_RETURN_IF_NOT_OK(adc_init(&cfg));
  }

  if (input_pin_bit_mask) {
    gpio_config_t cfg;
    cfg.pin_bit_mask = input_pin_bit_mask;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    AHOY_RETURN_IF_NOT_OK(gpio_config(&cfg));
  }

  if (output_pin_bit_mask) {
    gpio_config_t cfg;
    cfg.pin_bit_mask = output_pin_bit_mask;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    AHOY_RETURN_IF_NOT_OK(gpio_config(&cfg));
  }

  return ESP_OK;
}
